# 启动注册与预加载

默认入口为 `assets/content_registry.json`。根对象必须且只能包含 `bootstrap` 与 `manifests`；解析前会拒绝重复 JSON 属性，未知字段或目标文件不存在都会失败。

```json
{
  "bootstrap": {
    "app_config": "configs/global/app_config.json",
    "preload_manifest": "configs/manifests/preload_manifest.json"
  },
  "manifests": {
    "required": {
      "configs": "configs/manifests/config_manifest.json",
      "fonts": "configs/manifests/fonts_manifest.json",
      "audio": "configs/manifests/audio_manifest.json",
      "i18n": "configs/manifests/i18n_manifest.json",
      "textures": "configs/manifests/textures_manifest.json",
      "animations": "configs/manifests/animations_manifest.json",
      "effects": "configs/manifests/effects_manifest.json"
    },
    "additional": {}
  }
}
```

## `bootstrap`

| 字段 | 规则 |
| --- | --- |
| `app_config` | 必填 string；按 `assets/` 解析；Bootstrap 读取固定 AppConfig schema。 |
| `preload_manifest` | 必填 string；按 `assets/` 解析；用于启动纹理预加载。 |

`bootstrap` 不接受其他字段。通用 gameplay 配置不在启动阶段读取。

## `manifests.required`

`required` 必须是对象，且下列七项全部必填：

| 字段 | 目标 |
| --- | --- |
| `configs` | 通用 gameplay 配置 manifest；由内容加载阶段构建快照。 |
| `fonts` | 核心字体 manifest。 |
| `audio` | 核心 Sound/Music manifest。 |
| `i18n` | 国际化 manifest。 |
| `textures` | 核心纹理 manifest。 |
| `animations` | 核心 Animation manifest。 |
| `effects` | 核心 EffectDefinition manifest。 |

每个值必须是 string，按 `assets/` 解析后必须为普通文件。`ContentManifestPipeline` 读取这些声明；`GameContentLoader` 只在 Atlas、纹理、字体、音频、Animation 与 EffectDefinition 全部成功注册后发布 `configs` 生成的 `ConfigSnapshot`。

## `manifests.additional`

`additional` 可省略；存在时必须是 “module 名 → module manifest 路径” 的对象。module 名不决定 loader 类型，所有 module 都使用同一实体资源包 schema，并按名称稳定排序。详见[实体内容 module](entity-content.md)。

## `preload_manifest.json`

启动预加载当前只读取 `textures` 字符串数组：

```json
{ "textures": ["Akil_icon_1024.png", "start.png"] }
```

路径基于 `assets/preload/`，运行时 texture key 直接使用数组中的原字符串。图片解码、SDL texture 创建或重复 key 存储失败都会使预加载失败。
