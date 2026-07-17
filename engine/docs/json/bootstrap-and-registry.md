# 启动注册与预加载

默认入口为 `assets/content_registry.json`。Bootstrap 在程序启动时仅解析一次，并将解析后的只读 `ContentRegistry` 快照交给 engine Application 持有；Scene 通过 `SceneRuntimeContext` 借用该快照，内容加载阶段不会再次读取此文件。根对象必须且只能包含 `bootstrap` 与 `manifests`；解析前会拒绝重复 JSON 属性，未知字段或目标文件不存在都会失败。

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

启动预加载读取显式的 `textures` 条目，每项都包含稳定资源 key 和相对文件名：

```json
{
  "textures": [
    {
      "key": "moonline.brand.logo",
      "file": "Akil_icon_1024.png"
    }
  ]
}
```

项目纹理路径基于 `assets/preload/`，运行时使用条目的 `key`。Elysia Logo 由 engine 固定从 `assets/engine/preload/` 加载，不由项目 manifest 声明；缺失属于启动失败。项目 Logo 是可选资源，缺失时记录 warning 并跳过。

预加载纹理由 bootstrap 子系统自己的 `BootstrapTextureCache` 持有，不会注册到正式内容使用的 `ResourceManager`。因此 `GameContentLoader` 清理或重新加载游戏内容时不会影响启动画面。`get_preload_texture()` 返回借用指针，其有效期截止到 Application shutdown、bootstrap reset 或 SDL renderer 更换；退出 `StartupLoadingScene` 不会释放 engine 常驻 Logo，因此该内建场景可以安全再次进入。
