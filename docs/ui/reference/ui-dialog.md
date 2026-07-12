# UiDialog

头文件：`engine/ui/composites/ui_dialog.h`。可滚动正文的 overlay dialog。

调用：设置 `set_title_content`、`set_body_content`、`set_action_content`；先被 Window 拥有，再 `register_with_window(window, options)`；随后 `open`/`close`。`set_body_scroll_enabled`、`set_header_visible` 控制结构。窗口表面规则见[专题](../concepts/window-surfaces.md)。
