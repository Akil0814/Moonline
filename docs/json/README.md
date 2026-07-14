# 资源加载 JSON 文档

本目录记录 Moonline 当前已经实现的资源加载 JSON 规则。内容以现有 loader、资源请求构建器和运行时管理器为准，适合新增公共资源、角色或敌人时直接参考。

## 文档索引

- [启动注册与预加载](bootstrap-and-registry.md)：`content_registry.json`、核心/附加入口和 `preload_manifest.json`。
- [核心资源 manifest](core-resource-manifests.md)：字体、音频、纹理和国际化。
- [实体内容配置](entity-content.md)：角色/敌人 content manifest、entity manifest、纹理和音频 layout。
- [动画与特效](animation-and-effects.md)：核心/实体动画、Atlas、横向序列图和 EffectDefinition。

## 总加载链路

```text
assets/content_registry.json
  ├─ bootstrap.app_config ───────────> 窗口、渲染和音量启动配置
  ├─ bootstrap.preload_manifest ─────> assets/preload 下的启动纹理
  ├─ manifests.required
  │   ├─ fonts/audio/textures ───────> 核心资源请求
  │   ├─ animations/effects ─────────> Atlas、Animation、EffectDefinition 请求
  │   ├─ i18n ───────────────────────> LocalizationManager
  │   └─ configs ────────────────────> 当前只校验入口文件存在
  └─ manifests.additional
      ├─ characters ─────────────────> 角色实体资源
      └─ enemies ────────────────────> 敌人实体资源
```

资源计划的执行顺序是：Atlas 准备和提交、Animation 注册、EffectDefinition 注册，随后处理普通纹理、字体和音频。启动预加载和本地化有各自的加载阶段，不进入同一个 `ResourceLoadPlan`。

## 路径根目录

| 配置内容 | 相对路径根 |
| --- | --- |
| registry、content manifest 和 layout 中的 `configs/...` | `assets/` |
| 核心字体 manifest 的 `file` | `assets/fonts/` |
| 核心音频 manifest 的 `path` | `assets/audio/` |
| 核心纹理和动画 manifest 的 `path` | `assets/textures/` |
| preload manifest 的纹理路径 | `assets/preload/` |
| i18n manifest 的语言文件 | `assets/i18n/<language>/` |
| 实体 layout 中的动画/纹理路径 | 该实体解析后的 `texture_root` |
| 实体音频 layout 中的路径 | `<audio_root>/<asset_key>/` |

`PathManager` 对绝对路径保持绝对路径；以 `assets` 开头的相对路径基于项目根解析；以 `configs` 开头的路径基于 `assets` 解析。

## 通用约定

- JSON 文件必须可打开且语法有效；失败会终止对应加载阶段并写入日志。
- 文档中的“必填”表示 loader 当前会拒绝缺失或类型错误的字段。
- 文件存在性可能在入口解析、资源请求生成或实际提交阶段检查，具体以各文档说明为准。
- 运行时 key 是资源管理器的唯一查询标识；修改 key 会影响所有调用方。
- 本目录不描述 `app_config`、`input_config`、`game_config`、地图、关卡、角色数值、攻击和技能等业务 JSON 的内部结构。

