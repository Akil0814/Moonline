# 动画与特效 JSON

Animation 统一负责 Atlas 和 Animation 资源；Effect 只把已生成的 Animation 注册为 EffectDefinition。核心 manifest 与任意 additional module 最终进入同一请求计划和同一组运行时 registry。

Atlas 支持两种来源：

- `frame_directory`：配置给出帧数和文件前缀，严格生成预期文件名，不扫描目录。
- `horizontal_strip`：单张横向、单行、等宽、无间距的序列图。

## 加载顺序

```text
核心 animations_manifest.json
  -> AtlasBuildRequest + AnimationBuildRequest

每个 additional module 的 animations capability
  -> Animation layout + 每实体 animation config
  -> AtlasBuildRequest + AnimationBuildRequest

核心 effects_manifest.json
每个同 module 的 effects capability
  -> 只生成 AnimationEffectBuildRequest
```

请求组装会先收集核心与全部 module 的 Animation，再处理 Effect。运行时完成 Atlas 后注册所有 Animation，最后注册 EffectDefinition。

## 核心动画 manifest

`assets/configs/manifests/animations_manifest.json` 使用数组：

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

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `animations` | array<object> | 根对象中的必填数组 |
| `key` | string | 必填；点分 key 的每个 component 必须匹配 `[A-Za-z0-9_]+` |
| `path` | string | 必填、非空；基于 `assets/textures/` |
| `frame_count` | integer | 必填，必须大于 0 |
| `fps` | number | 必填，必须大于 0 |
| `loop` | boolean | 必填 |
| `horizontal_strip` | boolean | 可选，默认 `false` |
| `frame_prefix` | string | 目录帧必填；横向图禁止 |

目录帧示意：

```json
{
  "key": "example.ryougi_idle",
  "path": "character/RyougiShiki/animation/base/idle",
  "frame_prefix": "RyougiShiki_idle",
  "frame_count": 7,
  "fps": 10,
  "loop": true
}
```

`horizontal_strip: false` 时 `path` 必须是目录，并必须给出具体 `frame_prefix`；`true` 时 `path` 必须是普通图片文件且不能出现 `frame_prefix`。核心 Atlas/Animation 都使用 `key`。

## Additional module 的 Animation capability

```json
{
  "animations": {
    "texture_root": "textures/character/{asset_key}",
    "config_template": "configs/character/{asset_key}/animation_info.json",
    "frame_prefix_template": "{asset_key}_{animation}{segment_suffix}",
    "layouts": {
      "fighter": "configs/character/layouts/character_animation_layout.json"
    }
  }
}
```

`texture_root`、`config_template` 和非空 `layouts` 必填。`config_template` 必须含 `{asset_key}`。每个 entity 用自己的 `animation_layout` 选择 layout，再用自己的 config 描述实际存在的动画和播放参数。

`frame_directory` config 还必须提供 `frame_prefix_template`。模板：

- 只支持 `{asset_key}`、`{animation}`、`{segment_suffix}`。
- 必须含 `{asset_key}` 与 `{animation}`。
- config 中存在分段动画时必须含 `{segment_suffix}`。
- 必须是文件名前缀，不能含 `/`、`\` 或 `..`。
- 普通动画的 `{segment_suffix}` 替换为空；分段动画替换为 `_00` 至 `_99`。

横向图不使用文件前缀。

## Animation layout

layout 只描述相对于实体 `texture_root` 的路径：

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

动画逻辑名必须是合法 key component。每个条目必须且只能包含以下一种字符串字段：

- `path`：普通动画。
- `segment_path`：分段动画。

`segment_path` 含 `{segment}` 时替换该 token；不含时在路径末尾追加 segment 目录。两种情况都使用两位文件系统编号 `00` 至 `99`。

## Animation config

每个实体动画配置的根对象必须且只能包含 `defaults` 与 `animations`：

```json
{
  "defaults": {
    "source_type": "frame_directory"
  },
  "animations": {
    "idle": {
      "frame_count": 7,
      "fps": 10,
      "loop": true
    },
    "attack_normal": {
      "segments": [
        { "frame_count": 7, "fps": 10, "loop": false },
        { "frame_count": 8, "fps": 10, "loop": false }
      ]
    }
  }
}
```

`defaults` 必须且只能含 `source_type`，允许值为：

- `frame_directory`
- `horizontal_strip`

来源类型属于整个 config，不支持单个 animation/segment 覆盖。`animations` 必须是对象；属性名是合法动画逻辑 component，并且必须存在于选中的 layout。

普通动画条目必须且只能包含：

| 字段 | 类型 | 规则 |
| --- | --- | --- |
| `frame_count` | integer | 必填，大于 0 |
| `fps` | number | 必填，大于 0 |
| `loop` | boolean | 必填 |

分段动画条目必须且只能含非空 `segments` 数组；每个 segment 仍严格使用上述三个字段。一个动画最多 100 段，对应 index `0–99`。根级播放条目、单项来源覆盖以及遗漏播放字段的写法均无效。

## 分段编号与文件名

segment 的运行时 key 与文件系统编号有意分离：

```text
segment array index       0                    1
runtime key               entity.attack.0      entity.attack.1
layout directory          .../00                .../01
segment_suffix            _00                   _01
first frame filename      ..._00_000.png        ..._01_000.png
```

资源 key 永不补位；目录与 `segment_suffix` 固定补为两位。segment 超过 `99` 直接失败。帧 index 从 0 开始，文件侧至少补为三位：`_000.png`、`_001.png`、`_002.png`。

例如 RyougiShiki 的第一段普通攻击：

```text
source directory = assets/textures/character/RyougiShiki/animation/attack/normal/00
resource key     = ryougi_shiki.attack_normal.0
frame prefix     = RyougiShiki_attack_normal_00
first frame      = RyougiShiki_attack_normal_00_000.png
```

## `frame_directory`

Atlas 根据 `frame_count` 与前缀精确生成：

```text
<source>/<prefix>_000.png
<source>/<prefix>_001.png
...
```

每一帧产生一个准备任务。任意预期帧缺失都会失败；目录中多余 PNG 或其他文件不会被读取，也不会影响帧数。Atlas 不再收集、排序或回退扫描目录，因此不能依赖“目录中有多少 PNG 就加载多少帧”。

module 前缀由 `frame_prefix_template` 展开；核心目录动画直接使用 `frame_prefix`。两者都必须与磁盘文件名精确一致。

## `horizontal_strip`

module 横向图路径固定为：

```text
<texture_root>/<resolved layout path>/<animation logical name>.png
```

分段动画也先解析两位 segment layout 路径，再在对应目录中查找同名 `<animation>.png`。例如 FlyingDemon 的整个 animation config 使用 `defaults.source_type: "horizontal_strip"`，无需额外清单标记或单项覆盖。

横向图只解码并创建一份 SDL texture，再生成多个共享纹理的逻辑帧：

```text
frame_width = image_width / frame_count
source_rect = { frame_index * frame_width, 0, frame_width, image_height }
```

提交要求图片宽高和 `frame_count` 有效、图片宽度可被帧数整除且单帧宽度大于 0。它只支持单行、从左到右、等宽、无边距和无帧间距；不支持纵向、多行或不等宽帧。横向图只展开一个准备任务，逻辑帧通过 source rect 绘制。

## Module Animation 的 key 与路径

module 不再区分本体/特效 Animation 类型，也没有 `_effects_` 文件名硬编码：

```text
resource key = <entity id>[.<key_namespace>].<animation>[.<segment index>]
source path  = <texture_root>/<resolved layout path>
prefix       = 展开后的 frame_prefix_template（仅目录帧）
```

当前配置通过不同 module 表达职责：

```text
characters:
  namespace = ""
  key       = ryougi_shiki.attack_normal.0
  prefix    = RyougiShiki_attack_normal_00

character_effects:
  namespace = "effect"
  key       = ryougi_shiki.effect.attack_normal.0
  prefix    = RyougiShiki_effects_attack_normal_00
```

上面的 `effect` 和 `effects` 都来自 JSON namespace/template，而不是请求构建器特判。

## 核心 Effect manifest

`assets/configs/manifests/effects_manifest.json` 把核心 effect key 绑定到已生成的 Animation key：

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

| 字段 | 规则 |
| --- | --- |
| `key` | 必填；合法点分 key，成为 Effect registry key |
| `animation_key` | 必填；合法点分 key，必须对应计划中的 Animation |
| `default_width` / `default_height` | 可选，默认 0；必须同时为 0/省略或同时为正数 |
| `default_angle_degrees` | 可选 number，默认 0 |

## Module `effects` capability

`effects` capability 只提供每实体 config template：

```json
{
  "effects": {
    "config_template": "configs/character/{asset_key}/effect_info.json"
  }
}
```

每个 `effect_info.json` 只做逻辑映射：

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

- 根对象必须且只能包含 `effects` 对象。
- effect 名和 `animation` 值都是合法 key component；二者可以不同。
- `animation` 必须存在于同 module、同实体的 Animation config。
- 映射普通动画时生成一个 EffectDefinition；映射分段动画时为该逻辑动画已配置的每个 segment 展开一个定义。
- effect key 为 `<entity>[.<namespace>].<effect>[.<segment>]`。
- animation key 为 `<entity>[.<namespace>].<animation>[.<segment>]`。
- 默认宽高必须同时省略/为 0，或同时为正数；未设置时播放使用动画单帧自然尺寸。
- 创建 effect 时显式传入的尺寸优先于 EffectDefinition 默认尺寸，默认尺寸又优先于自然尺寸。

因此没有资源的动画或尾段应从 Animation config 省略；Effect config 不会补建 Atlas/Animation。当前 `character_effects` module 复用角色 entity manifest，但只加载自身 Animation config，再由 `effect_info.json` 注册定义。

## Key 冲突与来源

核心和所有 module 的请求完成后，会分别检查 Atlas、Animation、Effect 等 registry。Animation 通常同时生成同字符串的 Atlas key 和 Animation key，这是跨 registry 合法；但另一条核心或 module Animation 再生成同一个 Atlas/Animation key会失败。

错误会同时报告两个 `ResourceOrigin`，包括项目相对配置路径、JSON pointer、module/core、capability、entity、逻辑名和 segment。例如：

```text
Duplicate Animation key: ryougi_shiki.effect.attack_normal.0
  first:  assets/configs/.../effect_animation_info.json#/animations/attack_normal/segments/0
          scope=additional module=character_effects capability=animations entity=ryougi_shiki logical=attack_normal segment=0
  second: assets/configs/.../other.json#/animations/attack_normal/segments/0
          scope=additional module=other_effects capability=animations entity=ryougi_shiki logical=attack_normal segment=0
```

请求计划还会拒绝引用不存在 Atlas 的 Animation，以及引用不存在 Animation 的 Effect。

## 常见失败原因

- 核心 key 或任一逻辑 component 含空段、横线、空格、非 ASCII 字符等非法内容。
- 核心目录动画遗漏 `frame_prefix`，或横向图错误携带该字段。
- module 的 `frame_directory` 遗漏/误写 `frame_prefix_template` 或模板 token。
- Animation config 缺少 `defaults.source_type`，使用未知 source type 或单项覆盖。
- 普通/分段条目与 layout 的 `path`/`segment_path` 类型不匹配。
- segment 为空或超过 100 项，帧数/FPS 非正数，或 `loop` 类型错误。
- 预期的 `<prefix>_NNN.png` 缺失；额外 PNG 不会替代缺失帧。
- 横向图不是普通文件、宽度不能整除帧数或单帧尺寸无效。
- Effect 映射不存在的同 module Animation，默认尺寸只设置一边，或引用没有对应请求的 Animation。
- 同一 registry 的核心/module 请求生成重复 key；错误会列出 first/second 两个来源。
