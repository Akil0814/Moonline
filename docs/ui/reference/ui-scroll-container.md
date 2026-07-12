# UiScrollContainer

头文件：`engine/ui/containers/ui_scroll_container.h`。单一 content child 的可滚动 viewport。

调用：`set_content` 替换并接管 content；`clear_content` 移除。`set_scroll_axis`、`set_scrollbar_visibility`、`set_scroll_offset`、`set_scroll_step` 及 x/y 变体控制滚动。`scroll_offset`、`max_scroll_offset`、`viewport_rect`、`content_rect` 查询结果。

content 的 `content_extent()` 决定可滚动范围；不要把固定大高度当作内容测量替代品。
