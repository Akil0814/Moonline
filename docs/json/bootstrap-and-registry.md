# 启动注册与预加载

## `content_registry.json`

默认入口是 `assets/content_registry.json`。根对象必须且只能包含 `bootstrap` 和 `manifests`。

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
      "enemies": "configs/enemy/enemy_content_manifest.json"
    }
  }
}
```

### `bootstrap`

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `app_config` | string | 必填，路径必须指向已存在文件；内容由启动配置 loader 读取 |
| `preload_manifest` | string | 必填，路径必须指向已存在文件 |

`bootstrap` 不接受其他字段。本目录只记录 `app_config` 的入口关系，不覆盖其窗口、渲染和音量 schema。

### `manifests.required`

`required` 必须是对象，并且以下七个字段全部必填：

| 字段 | 目标 |
| --- | --- |
| `fonts` | 核心字体 manifest |
| `audio` | 核心声音和音乐 manifest |
| `i18n` | 国际化 manifest |
| `textures` | 核心纹理 manifest |
| `animations` | 核心动画 manifest |
| `effects` | 核心动画特效 manifest |
| `configs` | 通用配置入口 manifest |

每个值必须是字符串，按 `assets/` 解析后必须是普通文件。未知字段会失败。

当前资源管线会解析字体、音频、纹理、动画和 effect manifest；i18n 路径交给 `LocalizationManager`。`configs` 对应文件目前只在 registry 阶段检查存在，`ConfigLoadPipeline` 尚不读取其中的 `configs` 内容。

### `manifests.additional`

`additional` 可省略。存在时必须是“模块名到 manifest 路径”的对象，每个路径必须是字符串且文件存在。

当前只注册：

- `characters`
- `enemies`

其他模块名即使路径有效，也会在内容模块注册阶段失败。详情见[实体内容配置](entity-content.md)。

## `preload_manifest.json`

启动预加载目前只读取 `textures` 字符串数组：

```json
{
  "textures": [
    "Akil_icon_1024.png",
    "start.png"
  ]
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `textures` | array<string> | 必填；每项必须为非空字符串 |

每项路径基于 `assets/preload/`，运行时 texture key 直接使用数组中的原始字符串，例如 `start.png`。文件解码、SDL texture 创建或相同 key 存储失败都会使启动预加载失败。

当前 loader 不要求数组非空，也没有显式拒绝根对象中的其他字段；这些字段不会参与预加载。

## 常见失败原因

- registry 根对象缺少 `bootstrap` 或 `manifests`，或包含未知根字段。
- `bootstrap`、`required`、`additional` 不是对象。
- 必填入口缺失、不是字符串或目标文件不存在。
- `manifests` 含 `required`、`additional` 以外字段。
- `required` 含未知资源类别。
- `additional` 使用未注册的模块名。
- preload 的 `textures` 缺失、不是数组、含非字符串/空字符串，或图片无法加载。

