# UiChildHost

头文件：`engine/ui/core/ui_child_host.h`。拥有并更新 child 的基础容器。

特有公开 API：`add_child`、`create_child`、`extract_child`、`clear_children`、`child_count`、`child_at`、`move_child`、`set_child_layout_options`、`set_padding`、`set_clip_children`、`mark_layout_dirty`、`update_layout_if_dirty`。调用规则见[子项与布局](../concepts/child-layout.md)。

`add_child` 与 `create_child` 的返回值均为借用指针；生命周期仍属于 host。
