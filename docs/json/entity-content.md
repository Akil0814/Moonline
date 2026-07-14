# 实体内容配置

角色和敌人共用 `AnimatedEntityContentLoader`。入口来自 `content_registry.json` 的 `manifests.additional.characters` 和 `manifests.additional.enemies`。

## Content manifest

```json
{
  "entities": "configs/character/characters_manifest.json",
  "resources": {
    "texture_root": "textures/character/{asset_key}",
    "animation_config_template": "configs/character/{asset_key}/animation_info.json",
    "effect_animation_config_template": "configs/character/{asset_key}/effect_animation_info.json",
    "effect_info_template": "configs/character/{asset_key}/effect_info.json",
    "audio_root": "audio/character"
  },
  "capabilities": {
    "animations": {
      "layouts": {
        "fighter": "configs/character/layouts/character_animation_layout.json"
      }
    },
    "textures": { "layout": "configs/character/layouts/character_texture_layout.json" },
    "audio": { "layout": "configs/character/layouts/character_audio_layout.json" },
    "effects": {
      "layouts": {
        "fighter": "configs/character/layouts/character_effect_animation_layout.json"
      }
    }
  }
}
```

根对象只允许 `entities`、`resources`、`capabilities`。`entities` 必填且必须指向存在的 entity manifest；另外两项可省略。省略整个 `capabilities` 时只加载实体名册，不生成该模块的资源请求。

### `resources`

| 字段 | 何时需要 | 规则 |
| --- | --- | --- |
| `texture_root` | animations、effects 或 textures capability 存在 | 字符串；解析结果必须是目录 |
| `animation_config_template` | animations 存在 | 字符串，必须包含 `{asset_key}` |
| `effect_animation_config_template` | effects 存在 | 字符串，必须包含 `{asset_key}` |
| `effect_info_template` | effects 存在 | 字符串，必须包含 `{asset_key}` |
| `audio_root` | audio 存在 | 路径必须是已存在目录 |

模板替换后按配置路径解析，目标必须是普通文件。`resources` 中的未知字段会失败。

`texture_root` 的实体化规则：

- 含 `{asset_key}`：替换该标记。
- 不含标记：自动在路径末尾追加 `asset_key`。

例如 `textures/enemy/normal` 配合 `Slime` 会得到 `assets/textures/enemy/normal/Slime`。

### `capabilities`

只允许 `animations`、`effects`、`textures`、`audio`。

- `animations` 与 `effects` 必须且只能包含 `layouts` 对象。
- `textures` 与 `audio` 必须且只能包含单个 `layout` 路径。
- layout 路径基于 `assets/`，并且必须指向存在文件。
- animations/effects 的 `layouts` 是“布局名到路径”的映射；entity 的 `animation_layout` 必须同时存在于启用的对应映射中。

动画和 effect 详细格式见[动画与特效](animation-and-effects.md)。

## Entity manifest

```json
{
  "entities": [
    {
      "id": "slime",
      "asset_key": "Slime",
      "enabled": true,
      "animation_layout": "normal",
      "horizontal_strip": false
    }
  ]
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `entities` | array<object> | 根对象中的必填数组 |
| `id` | string | 必填、非空；构成运行时 key |
| `asset_key` | string | 必填、非空；用于路径和模板替换 |
| `enabled` | boolean | 可选，默认 `true`；为 `false` 时完全跳过该实体 |
| `animation_layout` | string | animations/effects 启用时必须匹配其 layout 名 |
| `horizontal_strip` | boolean | 可选，默认 `false`；只控制该实体的本体动画 |

当前 loader 没有显式检查不同条目的 `id` 或 `asset_key` 是否重复；新增内容应保证唯一，避免后续资源 key 冲突。

## 纹理 layout

```json
{
  "stand": "textures/stand.png",
  "selecting_icon": "textures/selecting_icon.png"
}
```

根对象的属性名是逻辑纹理名，值必须是相对于实体 `texture_root` 的字符串路径。

- 目标是普通文件：生成一个 texture request，key 为 `<entity id>.<layout key>`。
- 目标是目录：读取目录直属层级的所有普通文件，按完整路径排序；每个文件生成 key `<entity id>.<layout key>.<file stem>`。
- 目标不存在、目录为空或任一请求无效时失败。
- 目录模式会接收所有普通文件，不按扩展名过滤，也不会递归子目录。

## 音频 layout

```json
{
  "enter": "enter.wav",
  "selected": "selected.wav"
}
```

根对象的属性名是逻辑声音名，值必须是字符串。最终文件路径和 key 为：

```text
file = <audio_root>/<asset_key>/<layout path>
key  = <entity id>.<layout key>
```

实体音频会在请求生成阶段检查 key/path 非空以及文件存在。当前只生成 Sound 请求，不生成实体专属 Music 请求。

## 复用检查清单

新增实体时至少确认：

1. entity manifest 中 `id`、`asset_key` 唯一且 `enabled` 为预期值。
2. 所用 `animation_layout` 已在每个启用的动画/effect capability 中注册。
3. 所有包含实体差异的配置模板都保留 `{asset_key}`。
4. `texture_root`、`audio_root`、layout 和实体配置文件实际存在。
5. 纹理与声音运行时 key 不会与已有实体资源冲突。

## 常见失败原因

- content manifest 含未知根字段、resource 或 capability。
- capability 使用错误的 `layout`/`layouts` 形式。
- 启用资源 capability 却遗漏对应根目录或配置模板。
- 模板没有 `{asset_key}`，或替换后的配置文件不存在。
- entity 缺少 `id`/`asset_key`，字段类型错误或值为空。
- `animation_layout` 缺失，或没有在所用 capability 中注册。
- 纹理目标既不是文件也不是目录，或实体音频文件不存在。

