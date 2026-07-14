# 动画 JSON 配置规则

本文记录 Moonline 当前已经实现的动画资源 JSON 格式、路径解析方式和校验规则。内容以现有 loader、资源请求构建器和 Atlas 加载流程为准。

Atlas 支持目录帧和单行横向序列图。横向序列图必须由等宽、无间距的帧从左到右排列，不支持纵向、多行、边距或不等宽帧。

## 1. 加载链路

动画配置有两条入口。

### 1.1 核心动画

```text
assets/content_registry.json
  -> manifests.required.animations
  -> animations_manifest.json
  -> AtlasBuildRequest + AnimationBuildRequest
```

核心动画适合不依附于角色或敌人的公共动画。`content_registry.json` 中的 `manifests.required.animations` 必须是一个存在的文件路径。

### 1.2 角色和敌人动画

```text
assets/content_registry.json
  -> manifests.additional.characters / enemies
  -> *_content_manifest.json
      -> entities: *_manifest.json
      -> resources.animation_config_template: *_animation_info.json
      -> capabilities.animations.layouts: *_animation_layout.json
  -> AtlasBuildRequest + AnimationBuildRequest
```

实体 manifest 决定加载哪些实体以及使用哪个动画布局；布局决定动画名对应的资源相对路径；每个实体自己的 animation info 决定帧数和播放参数。

`manifests.additional` 当前只识别 `characters` 和 `enemies`。未注册的模块名会导致配置加载失败。

## 2. 核心动画 manifest

当前文件：`assets/configs/manifests/animations_manifest.json`。

最小有效示例：

```json
{
  "animations": [
    {
      "key": "test.animation",
      "path": "test/frame_group.png",
      "frame_count": 14,
      "fps": 10,
      "loop": true,
      "horizontal_strip": true
    }
  ]
}
```

字段规则：

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `animations` | array | 根对象中的必填数组 |
| `key` | string | 必填、非空，在该 manifest 内不可重复 |
| `path` | string | 必填、非空；相对于 `assets/textures/` 的目录或图片路径 |
| `frame_count` | integer | 必填，必须大于 0 |
| `fps` | number | 必填，必须大于 0 |
| `loop` | boolean | 必填 |
| `horizontal_strip` | boolean | 可选，默认 `false`；为 `true` 时 `path` 指向横向序列图 |

请求构建阶段根据 `horizontal_strip` 严格检查路径类型：`false` 要求目录，`true` 要求普通文件。Atlas key 和 animation key 都使用 `key`。

## 3. 实体 content manifest

角色和敌人使用相同的 content manifest 结构。以敌人为例：

```json
{
  "entities": "configs/enemy/enemy_manifest.json",
  "resources": {
    "texture_root": "textures/enemy/normal",
    "animation_config_template": "configs/enemy/normal/{asset_key}_animation_info.json"
  },
  "capabilities": {
    "animations": {
      "layouts": {
        "normal": "configs/enemy/enemy_general_animation_layout.json"
      }
    }
  }
}
```

根对象只允许 `entities`、`resources`、`capabilities`。

### 3.1 `entities`

- 必填字符串，指向存在的 entity manifest。
- 路径按 assets 根目录解析。

### 3.2 `resources`

只允许以下字段：

| 字段 | 用途 |
| --- | --- |
| `texture_root` | 动画或纹理 capability 存在时必填 |
| `animation_config_template` | 动画 capability 存在时必填，而且必须包含 `{asset_key}` |
| `audio_root` | 音频 capability 使用的根目录 |

`texture_root` 的实体化规则：

- 包含 `{asset_key}`：使用实体的 `asset_key` 替换该标记。
- 不包含 `{asset_key}`：在路径末尾自动追加 `asset_key`。
- 解析结果必须是已存在的目录。

例如，敌人的 `texture_root` 是 `textures/enemy/normal`，`asset_key` 是 `Slime`，最终纹理根目录为：

```text
assets/textures/enemy/normal/Slime
```

角色配置使用 `textures/character/{asset_key}`，替换后得到同样的实体专属目录。

### 3.3 `capabilities.animations.layouts`

`layouts` 是“布局名到布局 JSON 路径”的对象：

```json
{
  "animations": {
    "layouts": {
      "normal": "configs/enemy/enemy_general_animation_layout.json"
    }
  }
}
```

- 每个值必须是字符串并指向存在的文件。
- entity manifest 中的 `animation_layout` 必须能在这里找到。
- 使用 `effects` capability 时必须同时启用 `animations`。

## 4. Entity manifest

最小示例：

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

字段规则：

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `entities` | array | 根对象中的必填数组 |
| `id` | string | 必填、非空；生成运行时资源 key 的实体部分 |
| `asset_key` | string | 必填、非空；用于目录和配置模板替换 |
| `enabled` | boolean | 可选，默认 `true`；为 `false` 时跳过该实体 |
| `animation_layout` | string | 使用动画 capability 时必须提供，并且必须匹配已注册布局 |
| `horizontal_strip` | boolean | 可选，默认 `false`；为 `true` 时该实体的全部动画使用横向序列图 |

实体级 `horizontal_strip` 不支持在 animation info 中逐项覆盖，也不改变实体 effect layout 的目录帧规则。

以 Slime 为例：

```text
id                 = slime
asset_key          = Slime
animation_layout   = normal
animation info     = assets/configs/enemy/normal/Slime_animation_info.json
texture root       = assets/textures/enemy/normal/Slime
```

## 5. Animation layout

布局把逻辑动画名映射到实体纹理根目录下的相对路径。

### 5.1 普通动画路径

```json
{
  "animations": {
    "idle": { "path": "idle" },
    "move": { "path": "move" }
  }
}
```

### 5.2 分段动画路径

```json
{
  "animations": {
    "attack_normal": {
      "segment_path": "animation/attack/normal/{segment}"
    }
  }
}
```

每个动画条目必须且只能包含 `path`、`segment_path` 之一，并且对应值必须是字符串：

- 普通 animation info 条目要求布局提供 `path`。
- 含 `segments` 的 animation info 条目要求布局提供 `segment_path`。
- `segment_path` 包含 `{segment}` 时替换该标记。
- 不含 `{segment}` 时，在路径末尾追加段编号目录。
- JSON 数组下标从 0 开始，但资源目录段编号从 1 开始。

例如第一个分段解析为：

```text
animation/attack/normal/{segment}
-> animation/attack/normal/1
```

## 6. Animation info

animation info 是每个实体独立的播放配置。根对象的属性名必须能够在选定的 animation layout 中解析，除非该条目提供 `override_path`。

### 6.1 普通动画

```json
{
  "idle": {
    "frame_count": 14,
    "fps": 10,
    "loop": true
  }
}
```

### 6.2 分段动画

```json
{
  "attack_normal": {
    "segments": [
      { "frame_count": 7, "fps": 10, "loop": false },
      { "frame_count": 8, "fps": 10, "loop": false }
    ]
  }
}
```

每个普通条目或 segment 都必须提供：

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `frame_count` | integer | 必填，必须大于 0 |
| `fps` | number | 必填，必须大于 0 |
| `loop` | boolean | 必填 |
| `override_path` | string | 可选；覆盖 layout 中解析出的路径 |

`segments` 必须是非空数组，数组中的每一项必须是对象。分段动画的运行时 key 会追加从 0 开始的段索引，例如：

```text
ryougi_shiki.attack_normal.0
ryougi_shiki.attack_normal.1
```

`override_path` 对普通条目和单个 segment 都有效；它直接提供相对于实体纹理根目录的路径。

## 7. 实体动画的路径和 key 合成

普通敌人动画示例：

```text
entity id       = slime
asset key       = Slime
layout path     = idle
texture root    = assets/textures/enemy/normal/Slime

frame directory = assets/textures/enemy/normal/Slime/idle
atlas key       = slime.idle
animation key   = slime.idle
```

横向序列图敌人示例：

```text
entity id       = flying_demon
asset key       = FlyingDemon
layout path     = idle
horizontal strip = true

source image    = assets/textures/enemy/normal/FlyingDemon/idle/idle.png
atlas key       = flying_demon.idle
animation key   = flying_demon.idle
```

实体横向序列图固定按 `<实体纹理根>/<layout path>/<animation name>.png` 解析。分段路径会先按现有 `{segment}` 规则解析，再在对应目录内查找 `<animation name>.png`。

角色分段动画示例：

```text
entity id       = ryougi_shiki
asset key       = RyougiShiki
segment path    = animation/attack/normal/{segment}
segment index   = 0

frame directory = assets/textures/character/RyougiShiki/animation/attack/normal/1
atlas key       = ryougi_shiki.attack_normal.0
animation key   = ryougi_shiki.attack_normal.0
```

## 8. Effect 配置

### 8.1 核心 effect manifest

`effects_manifest.json` 把 effect key 绑定到已经注册的 animation key：

```json
{
  "effects": [
    {
      "key": "test.effect",
      "animation_key": "test.animation",
      "default_width": 128,
      "default_height": 128,
      "default_angle_degrees": 0
    }
  ]
}
```

- `key`、`animation_key` 必须是非空字符串，`key` 不可重复。
- 默认宽高可省略，此时都为 0；若提供尺寸，宽高必须同时为正数。
- `default_angle_degrees` 可省略，默认 0。

### 8.2 实体 effect layout

普通 effect 在同一条目中提供路径和播放参数：

```json
{
  "effects": {
    "attack_ranged_ground": {
      "path": "animation/effects/attack/ranged/ground",
      "frame_count": 1,
      "fps": 10,
      "loop": false
    }
  }
}
```

分段 effect 使用 `segment_path` 和非空 `segments` 数组：

```json
{
  "effects": {
    "attack_normal": {
      "segment_path": "animation/effects/attack/normal/{segment}",
      "segments": [
        { "frame_count": 3, "fps": 10, "loop": false }
      ]
    }
  }
}
```

规则如下：

- 普通 effect 必须包含 `path`、`frame_count`、`fps`、`loop`，不可同时包含 `segment_path` 或 `segments`。
- 分段 effect 必须包含 `segment_path` 和非空 `segments`，顶层不可再包含 `frame_count`、`fps`、`loop`。
- 每项 playback 的 `frame_count` 和 `fps` 必须大于 0。
- `default_width` 和 `default_height` 可省略；提供时必须同时为正数。
- `default_angle_degrees` 可选，默认 0。
- effect 名通过同名 animation info 条目关联；布局中没有对应 effect 时不生成 effect。
- 分段 effect 的 playback 数量不足时，缺少配置的动画段会被跳过。
- effect 目录内存在 `.no_effects` 文件时，该 effect 不生成资源请求。

## 9. Atlas 图片来源规则

### 9.1 目录帧

目录帧动画最终必须解析到一个存在的目录。

实体动画会优先检查标准命名序列：

```text
<asset_key>_<animation_name>_000.png
<asset_key>_<animation_name>_001.png
...
```

实体 effect 使用：

```text
<asset_key>_effects_<animation_name>_000.png
...
```

分段动画的标准文件名前缀会追加从 1 开始的段编号。如果目录中的 `_000.png` 符合标准前缀，Atlas 将按该前缀和三位数字编号严格查找全部帧，缺少任意帧都会失败。

如果未识别到标准前缀，则 Atlas：

1. 只收集目录直属层级中的 `.png` 文件，扩展名大小写不敏感。
2. 按完整文件名的字符串顺序排序。
3. 要求收集到的 PNG 数量与 `frame_count` 完全一致。

因此建议帧编号使用固定宽度，例如 `_000`、`_001`、`_002`，避免字符串排序与数值顺序不一致。

目录模式不会递归查找子目录。

### 9.2 横向序列图

横向模式只解码并创建一份纹理，然后按 `frame_count` 生成多个共享纹理的逻辑帧。每帧区域为：

```text
frame_width = image_width / frame_count
source_rect = { frame_index * frame_width, 0, frame_width, image_height }
```

横向序列图必须满足：

- 图片宽高均大于 0。
- `frame_count` 大于 0 且不超过图片宽度。
- 图片宽度能被 `frame_count` 整除。
- 所有帧等宽、占满图片高度、没有外边距和帧间距。

## 10. 常见失败原因

- manifest、layout 或 animation info 路径不存在。
- entity 的 `animation_layout` 没有在 content manifest 中注册。
- animation info 中的动画名不存在于 layout，且没有 `override_path`。
- 普通动画配到了 `segment_path`，或分段动画配到了 `path`。
- `frame_count` 为 0、`fps` 不大于 0，或者 `loop` 不是 boolean。
- `segments` 为空或包含非对象项。
- 最终路径类型与 `horizontal_strip` 不匹配。
- PNG 帧数量与 `frame_count` 不一致。
- 标准命名序列中存在断号或缺帧。
- 横向序列图宽度不能被 `frame_count` 整除。
