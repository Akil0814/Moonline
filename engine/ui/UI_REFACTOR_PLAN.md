# UI Refactor Plan

## Summary
当前 `engine/ui` 已进入“组件骨架逐步稳定、通用能力继续下沉”的阶段。  
这份文档用于记录已经完成的 UI 改造方向，以及后续几轮建议继续推进的计划，避免窗口、容器、布局、动画、输入各自演化出第二套语义。

---

## Current Direction

### 1. Core Rules
- `UiElement` 作为所有 UI 组件的基础几何与透明度入口。
- `screen_rect` 是唯一最终几何真相，不保存额外的运行时 center/layout 状态。
- `opacity` 采用单通道设计，不引入 local animation alpha。
- 父级 `opacity / clip` 通过 render command range 后处理叠加，不直接回写 child widget 状态。

### 2. Construction Rules
- 通用 centered 构造语义统一为 `from_center`。
- 常见控件优先同时提供：
  - `Rect`
  - `(position,size)`
  - `(center,size,from_center)`

### 3. Composition Preference
- 复杂控件优先拆成“原子控件 + 组合控件”。
- 通用状态推进、几何、输入、动画计算尽量下沉为可复用模块。
- 具体 widget 只保留自身对外 API 和业务语义，不重复维护第二套基础设施。

---

## Completed Refactors

### 1. `UiElement` opacity 下沉
- `UiElement` 提供统一 `set_opacity()` / `opacity()`
- `apply_opacity(UiRenderCommand&)`
- `apply_opacity(Color)`
- alpha 统一走乘法叠加

已接入方向：
- `UiImage`
- `UiFadeImage`
- `UiBlinkImage`
- `UiPulseImage`
- `UiButton`
- `UiBar`

### 2. `from_center` 通用化
- centered 输入语义已经从 image 专属命名统一上提。
- `UiImage / animated image / UiButton / UiLabel / UiBar` 已逐步接上 centered 构造。

### 3. image / label family 分组
- `widgets/image/`
- `widgets/label/`
- label family 已补齐：
  - `UiLabel`
  - `UiFadeLabel`
  - `UiBlinkLabel`
  - `UiPulseLabel`

### 4. opacity effect core 抽取
- `engine/ui/effects/`
  - `ui_opacity_common.h`
  - `ui_opacity_fade_core.h`
  - `ui_opacity_blink_core.h`
  - `ui_opacity_pulse_core.h`
- image / label 两侧统一采用“widget 内组合 core，再写回 `set_opacity(...)`”。

### 5. slider 组合化
- `UiSlider` 已改成组合控件：
  - `UiBar`
  - `UiDragHandle`
  - `UiLabel`
  - `UiNumber`
- value 显示统一走 `UiNumber`
- step 语义采用 `std::optional<float>`

### 6. `UiDragHandle` 抽取
- 已新增通用拖动原子控件 `UiDragHandle`
- 支持：
  - axis
  - drag bounds
  - color / texture visual
  - drag callbacks

### 7. container family 落地
- `UiContainer`
- `UiPanel`
- `UiGridContainer`
- `UiListContainer`
- `UiScrollContainer`

已确定规则：
- child ownership 归容器
- 递归 update / input / render 归容器
- `clip/opacity` 用 command range 统一后处理
- `UiScrollContainer` 当前采用显式 content size，不做自动测量

### 8. `UiWindow` 初版落地
- `UiWindow` 当前直接继承 `UiElement`
- 自带：
  - child tree
  - background / border / padding
  - focus registry
  - explicit neighbors
  - confirm / cancel
  - hover focus

### 9. 测试场景与滚轮链路修复
- 已有容器测试场景和菜单入口。
- 鼠标滚轮输入链已补齐坐标透传：
  - `SDL_MOUSEWHEEL -> RawInputEvent`
  - `RawInputEvent -> UiInputEvent`
- `UiScrollContainer` 的 hover wheel scroll 依赖这条链路。

---

## Current Pain Points

### 1. `UiWindow` 与 `UiContainer` 有重复骨架
当前重复点包括：
- child ownership
- layout dirty
- recursive update / frame / event dispatch
- child command range finalize
- destroyed child cleanup

### 2. layout 逻辑仍分散
当前至少分散在：
- `UiWindow::rebuild_layout()`
- `UiPanel::rebuild_layout()`
- `UiGridContainer::rebuild_layout()`
- `ui_container_shared_utils.*`

其中 anchored layout 已经出现重复实现倾向。

### 3. container / window / layout 边界还可继续收紧
当前“谁负责 child host，谁负责 layout，谁负责 focus/navigation”这三层已经基本成形，但共享骨架还没完全抽干净。

---

## Next Refactor Plan

### Plan A: 拆出共享 child-host 基类
建议新增一层轻语义基类，例如：
- `UiChildHost`
- 或 `UiCompositeElement`

职责只放：
- child ownership
- child layout options 存储
- layout dirty / `update_layout_if_dirty()`
- recursive update / frame / event dispatch
- child command range opacity / clip finalize
- destroyed child cleanup

不放：
- 具体布局策略
- focus / confirm / cancel
- theme 语义

建议继承关系：
- `UiContainer : UiChildHost`
- `UiWindow : UiChildHost`

这样可以减少 `UiWindow` 和 `UiContainer` 当前的重复代码，同时不把 `UiWindow` 强行语义化成“某种特殊布局容器”。

### Plan B: 拆出 layout 模块
建议新增 `engine/ui/layout/`，只放纯布局算法或轻量 driver。

第一批建议迁移：
- `UiLayoutAnchor`
- `UiLayoutPadding`
- `UiLayoutMargin`
- `UiLayoutChildOptions`
- `clamp_size(...)`
- `padded_content_rect(...)`
- `anchored_rect(...)`
- grid cell 对齐 helper

下一步可补：
- `layout_anchored_children(...)`
- `layout_grid_children(...)`
- `layout_list_children(...)`

目标：
- `UiWindow` 与 `UiPanel` 复用同一套 anchored layout
- `UiGridContainer` 不再维护第二套对齐算法
- layout 成为纯几何模块，不混入 input / focus / render

### Plan C: 明确 `UiWindow` 的上层职责
`UiWindow` 后续继续定位为“输入与焦点域窗口”，重点增强：
- focus target 批量注册
- 默认 focus 选择策略
- 更稳定的 mouse hover / pointer focus 行为
- 与 `UiScrollContainer` 的滚动协作
- 未来 modal / stack 的基础能力

不建议把复杂布局能力继续堆进 `UiWindow` 本体。

### Plan D: container family 继续扩展
后续可按一类容器负责一种排版继续补：
- `UiListContainer`
- `UiGridContainer`
- `UiScrollContainer`
- 未来可补 `UiStackContainer / UiDockContainer / UiWrapContainer`

原则：
- 每个 container 只关心一种布局策略
- 输入焦点与 confirm/cancel 仍交给更上层窗口语义

### Plan E: theme 托管与 layout 托管预留
未来可以支持：
- 组件或容器向 `UiThemeManager` 注册
- 容器托管 child 的位置、可见性、启用状态

但仍建议坚持：
- layout 只回写最终 rect
- theme 只回写视觉样式
- 不让 theme / layout 成为第二套运行时真相来源

---

## Recommended Immediate Order

### Step 1
先抽 `UiChildHost`
- 解决 `UiWindow / UiContainer` 重复骨架

### Step 2
再建 `engine/ui/layout/`
- 迁移 layout types 和 shared geometry/layout helpers

### Step 3
让以下类型统一复用 layout helper
- `UiWindow`
- `UiPanel`
- `UiGridContainer`
- `UiListContainer`

### Step 4
最后再增强 `UiWindow`
- focus registration convenience
- window 内多个 container 的导航协作
- scroll/focus 联动策略

---

## Non-Goals For Now
- 不引入通用 effect runner
- 不引入第二条 local animation alpha 通道
- 不引入自动几何导航推导
- 不引入完整约束式布局系统
- 不把 theme / layout / focus 三套系统揉成一个超大管理器

---

## Naming Notes
- 统一使用 `opacity`，避免混用 `alpha`
- centered 构造统一使用 `from_center`
- 若新增共享 child-host，命名更建议偏语义中立：
  - `UiChildHost`
  - `UiCompositeElement`

不建议继续扩大“Container”一词的语义边界，否则容易把 window、layout host、focus domain 都混成同一层概念。
