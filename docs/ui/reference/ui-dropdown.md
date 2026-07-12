# UiDropdown

头文件：`engine/ui/composites/ui_dropdown.h`。带 transient popup 的选择控件。

调用顺序：`set_options`/`add_option` 后 `set_selected_index`；将其纳入 Window child tree 后调用 `register_with_window(window)`；`open`、`close`、`toggle`、`is_open` 控制 popup。`set_on_selection_changed` 接收选中 index。注销使用 `unregister_from_window`。
