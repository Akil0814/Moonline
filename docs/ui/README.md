# Moonline UI 文档

本目录描述 `engine/ui` 的**当前** C++ UI 系统。它面向编写场景、控件和主题的
Moonline 开发者；SDL、ImGui 与 `engine/core` 的通用 API 不在本文档的参考范围内。

## 阅读路径

1. [使用指南](usage-guide.md)：先完成一个可输入、可渲染、可换主题的窗口。
2. [架构说明](architecture.md)：理解所有权、布局、输入和主题为什么这样协作。
3. API 参考：
   - [核心、布局、输入](api/core-layout-input.md)
   - [控件与视觉元素](api/widgets.md)
   - [容器与复合组件](api/containers-composites.md)
   - [样式、主题与窗口](api/style-window.md)

## 模块地图

| 目录 | 职责 | 场景代码通常是否直接使用 |
| --- | --- | --- |
| `core/` | 元素几何、控件状态、子节点所有权与渲染命令范围 | 是 |
| `layout/` | 锚点、列表和网格排版 | 是 |
| `widgets/` | 原子控件与视觉元素 | 是 |
| `containers/` | 布局、分组、滚动与页面承载 | 是 |
| `composites/` | 由多个 UI 元素装配出的可复用控件 | 是 |
| `input/`、`focus/` | 原始输入归一化、键盘/手柄导航 | 复杂界面时使用 |
| `style/`、`text/` | 主题、样式覆盖、视觉角色、本地化文本来源 | 是 |
| `window/` | 根窗口、Overlay、临时弹出层和 tooltip 协调 | 是 |
| `scroll/`、`effects/`、`number/` | 滚动状态、透明度动画、数值绘制基础设施 | 按需 |

## 术语

| 术语 | 含义 |
| --- | --- |
| `screen_rect` | 元素最终使用的屏幕坐标矩形；布局写入它，渲染和命中测试读取它。 |
| child host | `UiChildHost`：以 `std::unique_ptr` 持有子元素，并负责更新、输入和布局传播的节点。 |
| focus scope | 可在内部维护当前焦点控件的一块导航区域。窗口在 scope 之间导航，scope 在控件之间导航。 |
| visual role | 组件请求主题中某种语义样式（如 `Primary` 按钮），而不是直接绑定颜色。 |
| base style / overrides | 基础样式来自主题或调用方；覆盖层仅替换指定字段，并随主题重算未覆盖字段。 |
| overlay | 由 `UiWindow` 管理的可模态表面，如对话框；可拦截背景输入并恢复之前的焦点。 |
| transient popup | 非模态、由普通控件拥有的临时浮层，如下拉列表。 |

## 组件快速选择

| 需求 | 首选类型 |
| --- | --- |
| 普通可点击动作 | `UiButton`；成组选择用 `UiButtonGroup` |
| 布尔开关 / 单选 | `UiCheckbox` / `UiRadioButton`；带文字用 `UiLabeled…`；互斥集合用 `UiRadioGroup` |
| 数值调节或拖动 | `UiSlider` / `UiDragHandle` |
| 单行文本输入 | `UiTextInput` |
| 静态文字、图片、数字 | `UiLabel`、`UiTextBlock`、`UiImage`、`UiNumber` |
| 自动排版 | `UiListContainer`、`UiGridContainer`、`UiPanel`；任意锚定子项使用 `UiChildHost`/`UiWindow` |
| 内容溢出 | `UiScrollContainer` |
| 页签 | `UiTabContainer`；需要分开控制时用 `UiTabBar` + `UiTabView` |
| 确认或阅读对话框 | `UiConfirmationDialog` / `UiDialog` |
| 选择列表 | `UiDropdown` |
| 悬停说明 | `UiTooltip` |

## 公开头文件覆盖清单

API 参考以以下 77 个公开头文件为范围；每个头文件的类型、枚举、配置和公开调用入口都归入其
模块页面。此清单也用于后续 UI 新增 API 时的文档审查。

```text
core/ui_element.h                 core/ui_control.h
core/ui_child_host.h              core/ui_focusable.h
core/ui_radio_item.h              core/ui_render_command_range_utils.h
core/ui_text_align.h              layout/ui_anchor_layout.h
layout/ui_grid_layout.h           layout/ui_layout_geometry.h
layout/ui_layout_types.h          layout/ui_list_layout.h
input/ui_input_frame.h            input/ui_input_router.h
input/ui_input_state.h            input/ui_input_types.h
input/ui_gamepad_scroll_synthesizer.h
input/contracts/ui_input_event_receiver.h
input/contracts/ui_input_frame_receiver.h
focus/ui_control_focus_scope_host.h
focus/ui_delegated_focus_mixin.h  focus/ui_focus_scope.h
focus/ui_focus_scope_utils.h      scroll/ui_scroll_state.h
effects/ui_opacity_common.h       effects/ui_opacity_blink_core.h
effects/ui_opacity_fade_core.h    effects/ui_opacity_pulse_core.h
number/ui_digit_renderer.h        text/ui_text_content.h
text/ui_typography.h              style/ui_interaction_style.h
style/ui_palette.h                style/ui_style.h
style/ui_style_defaults.h         style/ui_theme.h
style/ui_theme_manager.h          style/ui_theme_style_resolver.h
style/ui_visual_roles.h           style/ui_visual_styles.h
widgets/ui_bar.h                  widgets/ui_button.h
widgets/ui_checkbox.h             widgets/ui_drag_handle.h
widgets/ui_radio_button.h         widgets/ui_slider.h
widgets/ui_text_input.h           widgets/image/ui_animation.h
widgets/image/ui_blink_image.h    widgets/image/ui_fade_image.h
widgets/image/ui_image.h          widgets/image/ui_pulse_image.h
widgets/label/ui_blink_label.h    widgets/label/ui_fade_label.h
widgets/label/ui_label.h          widgets/label/ui_pulse_label.h
widgets/number/ui_number.h        widgets/text/ui_text_block.h
containers/ui_button_group.h      containers/ui_chrome_container.h
containers/ui_grid_container.h    containers/ui_list_container.h
containers/ui_panel.h             containers/ui_radio_group.h
containers/ui_scroll_container.h  containers/ui_tab_view.h
composites/ui_confirmation_dialog.h composites/ui_dialog.h
composites/ui_dropdown.h          composites/ui_labeled_checkbox.h
composites/ui_labeled_radio_button.h composites/ui_tab_bar.h
composites/ui_tab_container.h     composites/ui_tooltip.h
window/ui_overlay.h               window/ui_transient_popup.h
window/ui_window.h
```
