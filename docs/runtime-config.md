# 运行时配置模块

Moonline 的运行时配置分成三个职责独立的模块：

| 模块 | 职责 | 启动时行为 |
| --- | --- | --- |
| `AppConfig` | 固定 schema 的程序默认值 | `AppConfigLoader` 直接加载 |
| `UserConfigService` | 玩家全局设置、运行时应用与可靠持久化 | 在 AppConfig 之后加载 |
| `ConfigService` | 发布通用游戏配置快照并提供强类型访问 | 快照构建成功后由 Bootstrapper 发布 |

## 启动顺序

```text
content_registry.json
  ├─ bootstrap.app_config
  ├─ bootstrap.game_config_manifest
  │    └─ ConfigLoadPipeline
  │         ├─ ConfigManifestLoader
  │         ├─ ConfigDocumentLoader
  │         └─ ConfigSnapshotBuilder
  └─ bootstrap.preload_manifest

AppConfig → 构建 ConfigSnapshot → UserConfig → publish ConfigSnapshot
```

加载器和 builder 只返回 `expected`，不写重复日志。启动失败由 Application/Bootstrap 启动边界统一记录。任何游戏配置失败都会阻止启动，且不会向 `ConfigService` 发布半成品。

## AppConfig

`assets/configs/global/app_config.json` 使用严格 version 1 schema：

```json
{
  "schema_version": 1,
  "window": {
    "title": "Moonline",
    "width": 1280,
    "height": 720,
    "fullscreen": false
  },
  "render": { "fps": 60, "vsync": true },
  "audio": {
    "master_volume": 100,
    "music_volume": 100,
    "sound_volume": 100
  },
  "localization": { "language": "en" }
}
```

缺失、未知、重复或非法字段都会使启动失败。宽高和 FPS 必须为正数，音量范围为 `0..100`，标题和语言不能为空。

## UserConfig

用户配置默认为 `player_data/user_config.json`。v1 保存完整的 window、render、audio 和 localization 快照，不保存窗口标题。

- 无 `schema_version` 的文件作为 v0 覆盖 AppConfig 默认值并迁移到 v1。
- 高于当前版本的文件原样保留并终止初始化。
- 加载恢复顺序为主文件、`.tmp`、`.bak`、AppConfig 默认值。
- 损坏主文件归档为带时间戳的 `.corrupt`。
- 保存通过 `.tmp` 重新解析验证、`.bak` 轮换和 rename 提交。

`UserConfigLoadResult` 分别报告 migrated、recovered、rebuilt 和 warning。VSync 继续使用重启标记，其他设置通过 `IUserConfigChangeHandler` 应用。

## 通用游戏配置

入口为 `assets/configs/manifests/config_manifest.json`：

```json
{
  "schema_version": 1,
  "configs": {
    "game": "configs/global/game_config.json"
  }
}
```

manifest 自身是严格 schema；`configs` 的属性名是 namespace，值是基于 `assets/` 解析的文档路径。当前只注册 `game_config.json`，InputConfig 和角色业务配置尚未注册。

被引用的文档没有业务 schema，根节点可以是 object、array、string、boolean 或 number，但不能是 `null`。根值注册为 namespace key；对象字段递归展开；数组索引使用 `.0/.1/...` 无补位 component。

```json
{
  "difficulty": { "enemy_health_scale": 1.0 },
  "spawn_points": [ { "x": 100, "y": 200 } ]
}
```

对应 key 包括：

```text
game
game.difficulty
game.difficulty.enemy_health_scale
game.spawn_points
game.spawn_points.0
game.spawn_points.0.x
```

namespace 和对象字段 component 必须符合 `[A-Za-z0-9_]+`。重复 JSON 属性、任意层级的 `null`、非法 component 和跨文档完整 key 冲突都会在快照发布前失败，并携带配置路径、JSON pointer、namespace 与 first/second 来源。

`ConfigService` 不接触文件系统或 JSON manifest，只提供：

```cpp
publish(snapshot);
contains(key);
get_int(key);       get_int_array(key);
get_double(key);    get_double_array(key);
get_bool(key);      get_bool_array(key);
get_string(key);    get_string_array(key);
get_vector2(key);   get_vector2_array(key);
get_rect(key);      get_rect_array(key);
```

Vector2、Rect、数组同质性和数值可表示性只在相应 getter 调用时校验。服务不暴露原始 JSON，不提供 optional、隐式 fallback、热重载或业务专用接口；角色、攻击、技能和关卡规则由 gameplay 层负责。

资源侧原 `loading::ConfigLoadPipeline` 已改名为 `ContentManifestPipeline`。它只加载核心资源 manifest 与 additional content module，不参与通用游戏配置快照构建。
