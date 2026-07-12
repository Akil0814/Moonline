# UiListContainer

头文件：`engine/ui/containers/ui_list_container.h`。沿主轴顺序排布 child 的焦点容器。

调用：`add_front`/`add_back` 添加 child；`set_direction` 选择 Vertical/Horizontal；`set_cross_align` 选择 Start/Center/End；`set_item_spacing` 设置间距。未设置 layout override 的 child 主轴尺寸来自 `content_extent()`。

适合表单、菜单、section body。固定 child slot 使用 `add_child(child, UiLayoutChildOptions{._use_size_override=true, ...})`。
