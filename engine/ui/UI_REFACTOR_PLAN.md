# UI 重构计划

## Summary

本文档用于记录 `engine/ui` 当前的 UI 架构方向，以及后续计划中的清理路径。

当前目录结构为：

* `core/`：`UiElement`、`UiControl`、`UiChildHost`
* `layout/`：布局类型、几何辅助函数、无状态布局驱动
* `containers/`：具体布局容器与滚动容器
* `window/`：焦点域窗口

## Current Direction

* `screen_rect` 是 widget 的唯一几何事实来源。
* `opacity` 保持单通道设计，并通过 render command 调制向下应用。
* 复杂控件优先使用组合，而不是深层继承。
* `UiChildHost` 负责 child tree 的所有权、递归 update / input / render、layout dirty 状态，以及 command range 的 clip / opacity 后处理。
* `layout/` 负责布局公式；containers 和 window 只选择布局驱动并提供配置，不重新实现具体数学逻辑。

## Completed Refactors

* `UiElement` 的 opacity 已统一，并下沉到基础 element 层。
* `from_center` 已成为通用的中心构造 tag。
* image 和 label widgets 已分别归入 `widgets/image/` 与 `widgets/label/`。
* 可复用的 opacity 动画核心已放入 `effects/`。
* `UiSlider` 已拆分为组合式 child widgets，并开始使用 `UiNumber`。
* `UiDragHandle` 已抽取为可复用的基础交互组件。
* `UiChildHost` 已替代旧的通用 container base，并迁入 `core/`。
* `UiWindow` 已复用 `UiChildHost` 处理 child tree hosting，自身只保留 focus / navigation 相关行为。
* 布局类型与无状态布局驱动已迁入 `layout/`。

## Current Pain Points

* `UiWindow` 仍然持有自定义的焦点域行为，后续很可能需要再进行一次清理。
* `UiScrollContainer` 仍然是布局特例，因为它同时混合了 viewport clipping 与 scroll offset。
* theme、focus、layout 目前仍刻意保持为相互独立的系统。

## Focus And Input Direction

* 焦点导航应该绑定 directional neighbors，而不是绑定具体按键。
* `up / down / left / right` 表示“下一个应该被选中的焦点目标”，并且应当独立于 keyboard、gamepad 或 mouse 的输入来源。
* `UiWindow` 目前仍作为焦点域 owner，继续负责解析：

  * 当前 focused target
  * 显式 neighbor navigation
  * `Confirm`
  * `Cancel`
  * 可选的 hover-to-focus
* 鼠标输入属于同一套交互模型：

  * 启用时，hover 可以移动 focus
  * click 应先让目标获得 focus，再把交互转发给目标
  * wheel routing 应继续流向 scroll-capable children
* `UiChildHost` 应继续负责 child ownership、recursive dispatch、layout、clipping 与 opacity，但不应默认变成通用 focus graph owner。
* 如果后续要把 focus 行为下沉，推荐方向是抽取专门的 focus-scope 或 interaction helper，而不是让每个 container 自己解释 keyboard / gamepad 输入。

## Planned API Direction

* 优先使用 “navigation neighbor” 注册，而不是 “bind key” API。
* v1 中只允许 `UiControl` 作为有效 focus target。
* 鼓励在 child creation 或 insertion 阶段进行便捷注册，例如：

  * 先用 layout options 创建 child，再注册 focus neighbors
  * 或者未来增加一个 overload，在同一步中同时接收 layout options 和 focus options
* v1 中 directional relationships 应保持显式注册；自动几何焦点推断继续延期。

## Recommended Next Steps

* 新布局算法继续加入 `layout/`，不要写进具体 containers 内部。
* 继续收窄 `containers/` 的职责，让每个具体 container 只选择布局驱动并提供 config。
* 后续再重新审视 `UiWindow` 的 focus registration 便利性，以及 scroll / focus 的协作方式。
* 决定下一轮清理是否要引入一个可复用的 focus-scope helper，并让它由 `UiWindow` 与未来的 focus-aware containers 共享。
* Render-command range 后处理逻辑放在 `core/ui_render_command_range_utils.*`。
