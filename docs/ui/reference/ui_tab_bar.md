# UiTabBar

头文件：`engine/ui/composites/ui_tab_bar.h`。只管理 tab button 列表的组件。

调用：`add_tab`、`extract_tab`、`clear_tabs` 管理项；`focused_index`/`selected_index` 查询；`set_focused_index`、`set_selected_index`、`clear_selection` 修改状态；`set_on_focus_changed`、`set_on_selection_changed` 接收 `optional<size_t>`。
