# UiWindow

头文件：`engine/ui/window/ui_window.h`。根 child host、窗口级焦点、overlay、popup 与 tooltip 协调器。

主要 API：

- 样式：`set_base_style`、`set_style_overrides`、`set_hover_focus_enabled`、`set_on_cancel`。
- 焦点：`register_focus_scope`、`unregister_focus_scope`、`set_scope_neighbors`、`set_focused_scope`、`focus_first_available_scope`。
- Overlay：`register_overlay`、`unregister_overlay`、`open_overlay`、`close_overlay`、`is_overlay_open`、`overlay_options`。
- Popup/tooltip：`register_transient_popup`、`unregister_transient_popup`、`activate_transient_popup`、`register_tooltip`、`unregister_tooltip`。

示例见[窗口表面专题](../concepts/window-surfaces.md)；Window 必须先拥有 dialog/overlay 元素再注册。
