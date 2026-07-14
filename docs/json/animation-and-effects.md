# 动画与特效 JSON

本文记录核心动画、实体动画、Atlas 图片来源和 Animation Effect 的当前规则。实体 content/entity manifest 的通用部分见[实体内容配置](entity-content.md)。

Atlas 支持目录帧和单行横向序列图。横向图只支持从左到右、等宽、无间距的帧，不支持纵向、多行、边距或不等宽帧。

## 加载链路

```text
核心动画 manifest
  -> AtlasBuildRequest + AnimationBuildRequest

实体 animation layout + 每实体 animation info
  -> AtlasBuildRequest + AnimationBuildRequest

实体 effect animation layout + 每实体 effect_animation_info
  -> AtlasBuildRequest + AnimationBuildRequest
  -> 每实体 effect_info
  -> AnimationEffectBuildRequest
```

Atlas 先完成构建，随后所有 Animation 注册到 `AnimationManager`，最后 EffectDefinition 注册到 `EffectManager`。

## 核心动画 manifest

`assets/configs/manifests/animations_manifest.json` 使用数组格式：

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

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `animations` | array<object> | 根对象中的必填数组 |
| `key` | string | 必填、非空、数组内不可重复；同时作为 Atlas/Animation key |
| `path` | string | 必填、非空，基于 `assets/textures/` |
| `frame_count` | integer | 必填，必须大于 0 |
| `fps` | number | 必填，必须大于 0 |
| `loop` | boolean | 必填 |
| `horizontal_strip` | boolean | 可选，默认 `false` |

`horizontal_strip: false` 要求 `path` 是目录；`true` 要求是普通文件。类型或文件状态不匹配会在请求生成阶段失败。

## Animation layout

实体动画 layout 把逻辑动画名映射到实体 `texture_root` 下的相对路径：

```json
{
  "animations": {
    "idle": { "path": "idle" },
    "attack_normal": {
      "segment_path": "animation/attack/normal/{segment}"
    }
  }
}
```

每个条目必须且只能包含 `path`、`segment_path` 之一，且值必须是字符串。

- 普通动画必须解析到 `path`。
- 分段动画必须解析到 `segment_path`。
- `segment_path` 含 `{segment}` 时替换标记；否则在末尾追加段编号目录。
- segment 数组索引从 0 开始，路径中的段编号从 1 开始。

```text
segment index 0
animation/attack/normal/{segment}
-> animation/attack/normal/1
```

## Animation info

每个实体的 animation info 记录实际帧数和播放参数。

普通动画：

```json
{
  "idle": {
    "frame_count": 14,
    "fps": 10,
    "loop": true
  }
}
```

分段动画：

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

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `frame_count` | integer | 普通条目或每个 segment 必填，必须大于 0 |
| `fps` | number | 普通条目或每个 segment 必填，必须大于 0 |
| `loop` | boolean | 普通条目或每个 segment 必填 |
| `segments` | array<object> | 分段动画使用，不能为空 |
| `override_path` | string | 可选，覆盖 layout 解析结果 |

动画名必须存在于 layout；提供 `override_path` 时可以绕过对应 layout 条目的查找。普通/分段形式必须与最终解析到的 `path`/`segment_path` 匹配。

## 实体动画路径和 key

目录帧实体动画：

```text
source    = <texture_root>/<resolved layout path>
atlas key = <entity id>.<animation>[.<segment index>]
anim key  = <entity id>.<animation>[.<segment index>]
prefix    = <asset_key>_<animation>[_<segment number>]
```

例如 RyougiShiki 第一段普通攻击：

```text
source    = assets/textures/character/RyougiShiki/animation/attack/normal/1
atlas key = ryougi_shiki.attack_normal.0
prefix    = RyougiShiki_attack_normal_1
```

实体级 `horizontal_strip: true` 控制该实体的全部本体动画，不支持在单个 animation info 条目覆盖。横向图路径固定为：

```text
<texture_root>/<resolved layout path>/<animation name>.png
```

特效动画始终使用目录帧，不受实体 `horizontal_strip` 影响。

## Atlas 目录帧规则

实体动画会先检查目录中是否存在标准序列的第 0 帧：

```text
<prefix>_000.png
```

存在时，Atlas 按同一前缀和三位数字编号严格生成 `frame_count` 个路径；任何断号或缺帧都会失败。

未识别到标准前缀时，Atlas 扫描目录：

1. 只收集直属层级 `.png` 文件，扩展名大小写不敏感。
2. 按完整文件名字符串排序。
3. 要求 PNG 数量与 `frame_count` 完全一致。

目录扫描不递归。建议始终使用 `_000`、`_001`、`_002` 等固定宽度编号。

## 横向序列图规则

横向图只解码并创建一份 SDL texture，再生成多个共享纹理的逻辑帧：

```text
frame_width = image_width / frame_count
source_rect = { frame_index * frame_width, 0, frame_width, image_height }
```

提交阶段要求：

- 图片和 `frame_count` 有效。
- 图片宽高大于 0，且 `frame_count` 不超过图片宽度。
- 图片宽度能被 `frame_count` 整除。
- 所有帧等宽、占满图片高度、没有外边距和帧间距。

横向图只展开一个准备任务，进度按一个工作单元计算；逻辑帧共享同一 texture，并通过 source rect 绘制。

## 核心 Effect manifest

`assets/configs/manifests/effects_manifest.json` 把 effect key 绑定到已经注册的 animation key：

```json
{
  "effects": [
    {
      "key": "effect.test",
      "animation_key": "test.animation",
      "default_width": 128,
      "default_height": 128,
      "default_angle_degrees": 0
    }
  ]
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `effects` | array<object> | 根对象中的必填数组 |
| `key` | string | 必填、非空、数组内不可重复 |
| `animation_key` | string | 必填、非空 |
| `default_width` | number | 可选，默认 0 |
| `default_height` | number | 可选，默认 0 |
| `default_angle_degrees` | number | 可选，默认 0 |

默认宽高必须同时为 0，或同时为正数。manifest loader 不验证 `animation_key` 是否已经注册；注册 EffectDefinition 时由 `EffectManager` 检查。

## 实体特效动画

实体 effect animation layout 仍使用通用 `AnimationLayout` 格式，根节点是 `animations`，只描述路径：

```json
{
  "animations": {
    "attack_normal": {
      "segment_path": "animation/effects/attack/normal/{segment}"
    },
    "attack_ranged_ground": {
      "path": "animation/effects/attack/ranged/ground"
    }
  }
}
```

每角色 `effect_animation_info.json` 完全复用普通 Animation info 格式：

```json
{
  "attack_normal": {
    "segments": [
      { "frame_count": 3, "fps": 10, "loop": false },
      { "frame_count": 3, "fps": 10, "loop": false }
    ]
  }
}
```

没有特效的动画或尾段直接省略，不生成 Atlas/Animation 请求。特效动画命名为：

```text
source        = <texture_root>/<resolved effect layout path>
animation key = <entity id>.effect.<animation>[.<segment index>]
prefix        = <asset_key>_effects_<animation>[_<segment number>]
```

## 实体 `effect_info.json`

该文件只把已经配置的特效动画注册为 EffectDefinition，不描述路径、帧数、FPS 或循环：

```json
{
  "effects": {
    "slash_trail": {
      "animation": "attack_normal",
      "default_width": 0,
      "default_height": 0,
      "default_angle_degrees": 0
    }
  }
}
```

- `effects` 必须是对象；逻辑 effect 名不能为空且不能重复。
- `animation` 必填、非空，并且必须存在于同角色的 `effect_animation_info.json`。
- effect 名与 animation 名可以不同。
- 映射到分段动画时，为已配置的每个 segment 自动生成 EffectDefinition。
- effect key 为 `<entity id>.effect.<effect>[.<segment index>]`。
- animation key 为 `<entity id>.effect.<animation>[.<segment index>]`。
- 默认宽高必须同时省略/为 0，或同时为正数。
- 未设置默认尺寸时，创建特效会使用动画当前帧的自然宽高。
- 创建特效时显式传入的 size 优先于 effect 默认尺寸，默认尺寸又优先于自然尺寸。

## 常见失败原因

- manifest、layout、animation info 或 effect info 不存在或 JSON 无效。
- 动画名不存在于 layout，且没有 `override_path`。
- 普通动画解析到 `segment_path`，或分段动画解析到 `path`。
- `frame_count` 为 0、`fps` 不大于 0、`loop` 不是 boolean，或 `segments` 为空。
- 最终路径类型与目录帧/横向图模式不匹配。
- PNG 数量与 `frame_count` 不一致，标准命名序列断号，或横向图宽度不能整除帧数。
- effect info 映射到不存在的特效动画，默认尺寸只设置一边，或包含重复逻辑 effect 名。
- EffectDefinition 引用的 Animation 请求不存在或尚未注册。

