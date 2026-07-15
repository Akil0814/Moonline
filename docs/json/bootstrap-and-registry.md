# 启动注册与预加载

## `content_registry.json`

默认入口是 `assets/content_registry.json`。根对象必须且只能包含 `bootstrap` 与 `manifests`：

```json
{
  "bootstrap": {
    "app_config": "configs/global/app_config.json",
    "preload_manifest": "configs/manifests/preload_manifest.json"
  },
  "manifests": {
    "required": {
      "fonts": "configs/manifests/fonts_manifest.json",
      "audio": "configs/manifests/audio_manifest.json",
      "i18n": "configs/manifests/i18n_manifest.json",
      "textures": "configs/manifests/textures_manifest.json",
      "animations": "configs/manifests/animations_manifest.json",
      "effects": "configs/manifests/effects_manifest.json",
      "configs": "configs/manifests/config_manifest.json"
    },
    "additional": {
      "characters": "configs/character/character_content_manifest.json",
      "character_effects": "configs/character/character_effect_content_manifest.json",
      "enemies": "configs/enemy/enemy_content_manifest.json"
    }
  }
}
```

registry 会在解析前拒绝重复 JSON 对象属性。

### `bootstrap`

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `app_config` | string | 必填；按 `assets/` 解析后必须是普通文件 |
| `preload_manifest` | string | 必填；按 `assets/` 解析后必须是普通文件 |

`bootstrap` 不接受其他字段。本目录只记录 `app_config` 的入口关系，不覆盖其窗口、渲染或音量 schema。

### `manifests.required`

`required` 必须是对象，且以下七项全部必填：

| 字段 | 目标 |
| --- | --- |
| `fonts` | 核心字体 manifest |
| `audio` | 核心 Sound/Music manifest |
| `i18n` | 国际化 manifest |
| `textures` | 核心纹理 manifest |
| `animations` | 核心动画 manifest |
| `effects` | 核心 EffectDefinition manifest |
| `configs` | 通用配置入口 manifest |

每个值必须是字符串，按 `assets/` 解析后必须是普通文件；未知 required 字段失败。资源管线会解析字体、音频、纹理、动画和 effect manifest；i18n 路径交给 `LocalizationManager`。`configs` 对应文件在资源管线中仍只要求存在。其 version 1 schema 已建立，并可由独立的通用 `ConfigService` 解析，但该服务尚未接入 Bootstrapper、GameContentLoader 或场景加载流程，详见[运行时配置模块](../runtime-config.md)。

### `manifests.additional`

`additional` 可省略。存在时必须是“module 名 → module manifest 路径”的对象：

```json
{
  "additional": {
    "character_effects": "configs/character/character_effect_content_manifest.json",
    "enemies": "configs/enemy/enemy_content_manifest.json"
  }
}
```

- module 名是任意 JSON 属性名，不再有 `characters`/`enemies` 白名单，也不决定 loader 类型。
- module 名只用于选择配置包、确定遍历顺序和记录 `ResourceOrigin`；它不会自动进入运行时资源 key。
- 路径必须是字符串，按 `assets/` 解析后必须是普通文件。
- module 使用统一的实体资源包 schema，见[实体内容 module](entity-content.md)。
- registry 把 module 保存为按名称排序的确定性集合，因此每次都以稳定顺序加载和组装请求。

同名 JSON 属性会被重复属性检查拒绝。不同 module 可以引用同一 entity manifest，但最终生成的同 registry key 仍必须唯一。

## `preload_manifest.json`

启动预加载当前只读取 `textures` 字符串数组：

```json
{
  "textures": [
    "Akil_icon_1024.png",
    "start.png"
  ]
}
```

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `textures` | array<string> | 必填；每项必须是非空字符串 |

每项路径基于 `assets/preload/`。运行时 texture key 直接使用数组中的原字符串，例如 `start.png`；它属于启动预加载流程，不经过实体 module 的 `ResourceKeyBuilder`。数组可以为空。当前 preload loader 会忽略根对象中的其他字段。

图片解码、SDL texture 创建或同 key 存储失败都会使启动预加载失败。

## 常见失败原因

- registry 根对象缺少 `bootstrap`/`manifests`，包含未知字段或重复对象属性。
- `bootstrap`、`required`、`additional` 类型错误。
- 必填入口缺失、不是字符串或目标文件不存在。
- `manifests` 或 `required` 含未知字段。
- additional module manifest 路径不是字符串或文件不存在。
- preload 的 `textures` 缺失、类型错误、含空字符串，或图片无法加载。
