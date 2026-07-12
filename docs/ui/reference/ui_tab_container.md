# UiTabContainer

头文件：`engine/ui/composites/ui_tab_container.h`。组合 TabBar 与 TabView 的完整页签组件。

调用：`add_tab(label, page)` 返回 `UiTabAddResult`，失败时从 `rejected_page` 取回所有权；`remove_tab` 返回 page 所有权；`clear_tabs` 清空。选择/焦点 API 与 TabBar 相同。
