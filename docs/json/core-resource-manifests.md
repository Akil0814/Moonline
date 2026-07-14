# 核心资源 Manifest

核心资源 manifest 由 `content_registry.json` 的 `manifests.required` 引用。动画和 effect 单独见[动画与特效](animation-and-effects.md)。

## 字体

```json
{
  "sizes": [10, 20, 30, 40, 50, 60, 70],
  "fonts": [
    { "key": "ui.latin", "file": "fusion-pixel.ttf" }
  ]
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `sizes` | array<integer> | 必须严格等于 `[10,20,30,40,50,60,70]`，顺序和数量都不能改变 |
| `fonts` | array<object> | 必填且不能为空 |
| `fonts[].key` | string | 必填、非空，在数组内不可重复 |
| `fonts[].file` | string | 必填、非空，基于 `assets/fonts/` |

每个字体会为七个尺寸分别生成请求，运行时 key 为 `<key>.<size>`，例如 `ui.latin.30`。manifest loader 不检查字体文件存在性，实际字体加载失败会在后续资源提交阶段报告。

## 音频

```json
{
  "sounds": {
    "system.confirm": { "path": "system/confirm.wav" }
  },
  "music": {
    "scene.main": { "path": "scene/main.ogg" }
  }
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `sounds` | object | 必填；属性名成为 Sound key |
| `music` | object | 必填；属性名成为 Music key |
| 条目 `path` | string | 必填，基于 `assets/audio/` |

两个对象都可以为空。请求生成阶段会拒绝空 key 和空路径，但不检查音频文件是否存在；文件读取和解码错误由后续音频加载阶段报告。对象属性应保持唯一，避免重复 JSON 属性在解析时被覆盖。

## 纹理

```json
{
  "textures": {
    "ui.moon": { "path": "ui/moon.png" }
  }
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `textures` | object | 必填 |
| 属性名 | string | 直接成为 texture key |
| 条目 `path` | string | 必填，基于 `assets/textures/` |

请求生成阶段要求 key 和路径非空，并要求解析结果是普通文件。目录不能作为核心 texture 条目。对象属性天然作为唯一 key；重复 JSON 属性不应使用。

## 国际化 manifest

```json
{
  "default_language": "en",
  "languages": ["en", "zh_cn", "ja"],
  "file": ["base.json"]
}
```

| 字段 | 类型 | 当前规则 |
| --- | --- | --- |
| `default_language` | string | 必填、非空 |
| `languages` | array<string> | 必填且不能为空 |
| `file` | array<string> | 必填且不能为空，每项必须非空 |

语言文件按 `assets/i18n/<language>/<file>` 查找；如果 `<language>` 目录不存在，还会尝试把语言名中的下划线替换为连字符。默认语言不在 `languages` 时，运行时会把它追加为受支持语言。

每个语言目录会依次加载 `file` 中的全部 JSON 并合并翻译表。文件不存在、JSON 无效或翻译数据形状不受支持都会使该语言加载失败。

## 校验时机摘要

| 类别 | manifest 阶段 | 请求/运行时阶段 |
| --- | --- | --- |
| 字体 | 固定尺寸、条目类型、非空/重复 key | 字体文件读取与字体创建 |
| 音频 | 分组、条目和 `path` 类型 | 音频文件读取与解码 |
| 纹理 | `textures` 和条目结构 | 非空 key/path、文件存在、图片解码和 texture 创建 |
| i18n | manifest 字段和列表类型 | 语言目录、翻译文件与翻译数据内容 |
