# 实体内容 module

`manifests.additional` 中的每个条目都是一个任意命名的实体资源包。所有包由同一个 loader 读取；module 名不会选择角色或敌人专用代码。

## Module manifest

下面是仓库当前 `character_effects` 的最小结构：

```json
{
  "entities": "configs/character/characters_manifest.json",
  "key_namespace": "effect",
  "capabilities": {
    "animations": {
      "texture_root": "textures/character/{asset_key}",
      "config_template": "configs/character/{asset_key}/effect_animation_info.json",
      "frame_prefix_template": "{asset_key}_effects_{animation}{segment_suffix}",
      "layouts": {
        "fighter": "configs/character/layouts/character_effect_animation_layout.json"
      }
    },
    "effects": {
      "config_template": "configs/character/{asset_key}/effect_info.json"
    }
  }
}
```

根对象必须且只能包含以下三个字段：

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `entities` | string | 必填；按 `assets/` 解析后必须是 entity manifest 文件 |
| `key_namespace` | string | 必填；可为 `""`，非空时必须是合法 key component |
| `capabilities` | object | 必填；可以为空，只允许四种固定 capability |

允许的 capability 为 `animations`、`effects`、`textures`、`audio`，每一种都可省略。未知 capability、未知 module 根字段或 capability 内未知字段都会失败。`effects` 存在时，同一 module 必须同时存在 `animations`；Effect 不会引用其他 module 的逻辑动画配置。

一个合法的只登记实体、不生成资源请求的 module 是：

```json
{
  "entities": "configs/enemy/enemy_manifest.json",
  "key_namespace": "",
  "capabilities": {}
}
```

## Capability schema

### `animations`

```json
{
  "animations": {
    "texture_root": "textures/enemy/normal/{asset_key}",
    "config_template": "configs/enemy/normal/{asset_key}_animation_info.json",
    "frame_prefix_template": "{asset_key}_{animation}{segment_suffix}",
    "layouts": {
      "normal": "configs/enemy/enemy_general_animation_layout.json"
    }
  }
}
```

| 字段 | 规则 |
| --- | --- |
| `texture_root` | 必填；实体化后必须是已存在目录 |
| `config_template` | 必填；必须包含 `{asset_key}`，替换后必须是配置文件 |
| `frame_prefix_template` | `frame_directory` 来源必填；`horizontal_strip` 来源不需要 |
| `layouts` | 必填、非空；合法 layout component 到现存 Animation layout 文件的映射 |

`frame_prefix_template` 的详细 token 和文件生成规则见[动画与特效](animation-and-effects.md#frame_directory)。

### `effects`

```json
{
  "effects": {
    "config_template": "configs/character/{asset_key}/effect_info.json"
  }
}
```

`config_template` 是唯一允许字段，必填且必须含 `{asset_key}`。该 capability 只把同 module 的已配置动画映射为 EffectDefinition，不描述图片、Atlas、帧数或 FPS。

### `textures`

```json
{
  "textures": {
    "texture_root": "textures/character/{asset_key}",
    "layout": "configs/character/layouts/character_texture_layout.json"
  }
}
```

`texture_root` 与 `layout` 都必填。前者实体化后必须是目录；后者按 `assets/` 解析后必须是现存文件。

### `audio`

```json
{
  "audio": {
    "audio_root": "audio/character/{asset_key}",
    "layout": "configs/character/layouts/character_audio_layout.json"
  }
}
```

`audio_root` 与 `layout` 都必填。实体化后的 audio root 必须是目录；当前 module Audio 只生成 Sound，不生成 Music。

## `{asset_key}` 与路径解析

`texture_root` 和 `audio_root` 使用相同的实体化规则：

- 模式包含 `{asset_key}` 时，替换全部标记。
- 模式不含标记时，在根路径末尾自动追加 `asset_key`。
- 只允许 `{asset_key}` token；未知 token、括号不匹配或实体化后目录不存在都会失败。

所有 `config_template` 必须显式包含 `{asset_key}`，替换后按配置路径解析并要求目标是普通文件。Animation 的 `layouts` 以及 Texture/Audio 的 `layout` 路径基于 `assets/`。

## Entity manifest

```json
{
  "entities": [
    {
      "id": "flying_demon",
      "asset_key": "FlyingDemon",
      "enabled": true,
      "animation_layout": "normal"
    }
  ]
}
```

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `entities` | array<object> | 根对象中的必填数组 |
| `id` | string | 必填；合法 key component，作为运行时 key 首段 |
| `asset_key` | string | 必填；合法 key component，用于路径和模板替换 |
| `enabled` | boolean | 可选，默认 `true`；`false` 时跳过该实体 |
| `animation_layout` | string | Animation capability 使用时必须存在并匹配 `layouts`；若提供也必须是合法 component |

entity 条目只接受上述字段。图片来源由每个 Animation config 的 `defaults.source_type` 统一决定，不在 entity 条目中配置。

同一 entity manifest 可以被多个 module 复用。例如 `characters` 使用空 namespace 加载本体资源，`character_effects` 使用 `effect` namespace 加载同一批角色的特效动画；职责和 key 空间由 module capability/namespace 分离。

## 资源 key

所有 capability 都调用同一个 key builder：

```text
base = <entity id>[.<key_namespace>]
Animation = <base>.<animation>[.<segment index>]
Effect    = <base>.<effect>[.<segment index>]
Texture   = <base>.<texture logical name>[.<directory file stem>]
Sound     = <base>.<audio logical name>
```

例如：

```text
characters / namespace ""       -> ryougi_shiki.idle
character_effects / "effect"     -> ryougi_shiki.effect.attack_normal.0
```

component 统一使用 `[A-Za-z0-9_]+`。`key_namespace: ""` 会被跳过，不生成空段；segment key 不补位。

## Texture layout

```json
{
  "stand": "textures/stand.png",
  "selecting_icon": "textures/selecting_icon.png"
}
```

属性名是纹理逻辑 component，值是相对于该实体 `texture_root` 的路径。

- 目标是文件：生成 `<base>.<layout key>`。
- 目标是目录：只枚举直属普通文件并按路径排序，生成 `<base>.<layout key>.<file stem>`。
- 目录枚举不递归，也不按扩展名过滤；目录必须非空。
- 每个目录文件的 stem 也必须满足 key component 语法，否则失败。

这里的目录枚举只属于普通 Texture capability。Animation 的 `frame_directory` 已完全禁止扫描，不能混淆。

## Audio layout

```json
{
  "enter": "enter.wav",
  "selected": "selected.wav"
}
```

属性名是 Sound 逻辑 component，值相对于实体化后的 `audio_root`：

```text
file = <audio_root>/<layout path>
key  = <entity id>[.<key_namespace>].<layout key>
```

目标必须是普通文件。

## 当前 module 划分

| module | namespace | capability |
| --- | --- | --- |
| `characters` | `""` | `animations`、`textures`、`audio` |
| `character_effects` | `effect` | `animations`、`effects` |
| `enemies` | `""` | `animations` |

这些名称和组合只是当前内容配置。新增 `npcs`、`boss_effects` 或其他名称不需要增加 loader 子类，只需遵守相同 schema。

## 常见失败原因

- module 缺少三个必填根字段或包含未知字段。
- capability 名未知、类型错误或包含未知字段。
- `effects` 没有同 module 的 `animations`。
- namespace、entity、layout 或逻辑名不符合 key component 语法。
- `{asset_key}` 模板无效、config template 缺少标记，或替换后的文件/目录不存在。
- entity 的 `animation_layout` 不在 Animation capability 的 `layouts` 中。
- Texture/Audio layout 无效、目标不存在，或目录纹理 file stem 非法。
- 核心或其他 module 在同一资源 registry 生成了相同 key；错误会同时列出两个 `ResourceOrigin`。
