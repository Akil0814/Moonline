# UiDragHandle

头文件：`engine/ui/widgets/ui_drag_handle.h`。受 bounds/axis 约束的可拖拽控件。

调用：`set_drag_handle_config` 原子配置；`set_drag_axis` 选择 Free/Horizontal/Vertical；`set_drag_bounds`/`clear_drag_bounds` 管理范围；`set_on_dragged`、`set_on_drag_ended` 接收 handle center。`begin_drag_from_pointer` 和 `cancel_drag` 适用于程序化控制。
