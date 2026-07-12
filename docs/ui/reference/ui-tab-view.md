# UiTabView

头文件：`engine/ui/containers/ui_tab_view.h`。只承载 tab page 的容器。

调用：`add_page` 接管页面；`extract_page` 返回页面所有权；`clear_pages` 清空；`set_selected_index`/`clear_selection` 切换可见页面。通常由 `UiTabContainer` 内部使用，只有需要拆分 TabBar/TabView 时直接使用。
