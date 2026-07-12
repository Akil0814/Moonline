# UiDialog

头文件：`engine/ui/composites/ui_dialog.h`。可滚动正文的 overlay dialog。

调用：设置 `set_title_content`、`set_body_content`、`set_action_content`；先被 Window 拥有，再 `register_with_window(window, options)`；随后 `open`/`close`。`set_body_scroll_enabled`、`set_header_visible` 控制结构。窗口表面规则见[专题](../concepts/window_surfaces.md)。

默认注册 options 来自 Dialog style 的 `overlay_defaults`；需要非默认 modal、placement 或 close policy 时，在 `register_with_window` 第二参数传 `UiOverlayOptions`。未注册或已从 Window 脱离时 `open()`/`close()` 安全无操作。
