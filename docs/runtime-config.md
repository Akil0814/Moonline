# 运行时配置模块

Moonline 的运行时配置分为三个职责明确的模块。

| 模块 | 用途 | 是否可写 | 是否接入启动流程 |
| --- | --- | --- | --- |
| `AppConfig` | 程序发布时提供窗口、渲染、音频和默认语言 | 否 | 是 |
| `UserConfigService` | 玩家全局设置、运行时应用、版本迁移与持久化 | 是 | 是 |
| `ConfigService` | 按任意 namespace 加载游戏数据，并提供强类型 key 访问 | 否 | 否，本轮只建立接口与 manifest schema |

## AppConfig

入口为 `assets/configs/global/app_config.json`，当前 schema version 为 1。根对象及所有子对象均为严格 schema：缺少字段、未知字段、重复属性或类型错误都会使启动失败。

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

宽高和 FPS 必须为正数，FPS 必须有限；音量范围为 `0..100`；标题和语言不能为空。旧字段 `default_width`、`default_height`、`default_fps` 不再接受。

`AppConfig` 保存窗口标题和一份 `UserConfigData` 默认值。`Bootstrapper` 加载用户设置后，将两者合成为最终只读的 `StartupSettings`。

## UserConfig

用户文件默认位于 `player_data/user_config.json`。v1 使用与 AppConfig 相同的 `window`、`render`、`audio`、`localization` 字段，但不保存窗口标题，并且始终保存完整快照。

- 无 `schema_version` 的旧文件视为 v0：合法字段覆盖 AppConfig 默认值，然后自动迁移并保存为 v1。
- v0 的空语言回退到 AppConfig 默认语言。
- 高于当前版本的文件会原样保留并终止初始化，不会自动降级或覆盖。
- 有效主文件优先；主文件损坏后依次尝试 `.tmp`、`.bak`，最后使用 AppConfig 默认值重建。
- 损坏主文件会改名为带毫秒时间戳的 `.corrupt` 文件。
- 保存先写 `.tmp`，关闭并重新严格解析，再轮换 `.bak` 并 rename 提交；提交失败时尝试恢复旧主文件。

`UserConfigLoadResult` 会分别报告 `migrated`、`recovered`、`rebuilt` 和 warning。VSync 变更仍标记为需要重启；其他现有设置继续通过 `IUserConfigChangeHandler` 应用到运行时。

## 通用 ConfigService

入口 manifest 为 `assets/configs/manifests/config_manifest.json`：

```json
{
  "schema_version": 1,
  "configs": {
    "game": "configs/global/game_config.json"
  }
}
```

`configs` 的属性名是文档 namespace，值是基于 `assets/` 的 JSON 路径。当前真实 manifest 只声明 `game`；InputConfig 和角色业务配置尚未注册。

完整 key 为 `<namespace>.<JSON path>`。对象字段与 namespace 的每个 component 都必须符合 `[A-Za-z0-9_]+`，点只连接 component；数组索引使用无补位十进制，例如 `game.spawn_points.0`。

初始化会索引对象、数组和叶子节点，并在全部文档成功后原子发布不可变 snapshot。首次失败保持未初始化；重新初始化失败保留旧 snapshot。`null`、重复 JSON 属性、非法 component 和完整 key 冲突都会在发布前失败。冲突错误包含 first/second 两个项目相对配置路径、JSON pointer、namespace 和完整 key。

公开 getter：

```cpp
get_int(key);       get_int_array(key);
get_double(key);    get_double_array(key);
get_bool(key);      get_bool_array(key);
get_string(key);    get_string_array(key);
get_vector2(key);   get_vector2_array(key);
get_rect(key);      get_rect_array(key);
```

- `get_int` 只接受可表示为 `int64_t` 的 JSON integer。
- `get_double` 接受 integer 或 floating number，但结果必须有限。
- `Vector2` 必须恰好为 `{x, y}`。
- `Rect` 必须恰好为 `{x, y, width, height}`，宽高不得为负。
- 几何分量必须是有限且可表示为 `float` 的数值。
- 数组逐项执行同一类型规则；空数组对任意数组 getter 都合法。

接口不暴露原始 JSON，也不提供 optional 或隐式 fallback。访问失败会返回包含 key、期望/实际类型和来源的 `ConfigAccessFailure`；服务内部按“错误类别 + key + 请求类型”线程安全地只记录一次错误日志。

目前资源管线仍只验证 `config_manifest.json` 文件存在，`Bootstrapper`、`GameContentLoader` 和场景都不会初始化通用 `ConfigService`。未来 gameplay 接入时负责角色、攻击、技能和关卡等业务约束。
