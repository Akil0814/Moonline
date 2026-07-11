# API：容器与复合组件

本页覆盖 `containers/` 和 `composites/`。容器接管 child 所有权；复合组件对外暴露一个
完整交互语义，内部按钮、标签和列表不是调用方应单独拥有的对象。

## 通用容器

| API | 含义 |
| --- | --- |
| `containers/ui_list_container.h` — `UiListDirection::{Vertical,Horizontal}` | 列表容器的公开方向枚举。 |
| `UiListContainer` | 可焦点的顺序容器。`add_back/add_front` 接收任意 `UiElement`；`set_direction/direction`、`set_item_spacing/item_spacing` 控制布局；`content_extent` 测量内容。其 update/input/render 与焦点 scope 行为继承 `UiControlFocusScopeHost`。 |
| `containers/ui_grid_container.h` — `UiGridContainer` | 可焦点网格。`add_child(unique_ptr)` 使用当前默认网格参数，带 `UiLayoutChildOptions` 的重载保留单项选项；`set_column_count/column_count`、`set_cell_spacing/cell_spacing`、`set_fill_by_row/fills_by_row` 配置网格；`content_extent` 返回所需范围。 |
| `containers/ui_panel.h` — `UiPanelInsertDirection` | `Left`、`Right`、`Up`、`Down` 指定新 child 相对上一次插入点的方向。 |
| `UiPanel` | 带背景/边框的可焦点面板。`add_child(unique_ptr,direction)` 按插入方向接收内容；`set_base_style/style/overrides/clear` 与 `set_visual_role/visual_role` 使用 `UiPanelStyle` 和 `UiPanelVisualRole`。`content_extent` 包含内容。 |
| `containers/ui_chrome_container.h` — `UiChromeActionInsertPosition` | `Header` 或 `Footer`，决定 action 插入到 chrome 的哪个区域。 |
| `UiChromeContainer` | 带 header 与 body 的可焦点容器。`add_left_action`、`add_title_child`、`add_right_action` 分别放入 header 槽位，配套 `clear_*` 清理；`set_body/body_content/clear_body` 管理唯一 body。`set_header_visible`、`set_header_height`、`set_header_padding`、`set_body_padding` 控制区域；`focus_body_first_available` 可显式进入委托 body scope；样式 API 使用 `UiChromeContainerStyle`。 |
| `containers/ui_button_group.h` — `UiButtonGroup` | `UiListContainer` 的按钮选择组。`add_button` 仅接收 `unique_ptr<UiButton>`；`selected_index`、`set_selected_index`、`clear_selection` 管理选择；`set_on_selection_changed` 接收可选 index。组会保留调用方已有按钮样式覆盖。 |
| `containers/ui_radio_group.h` — `UiRadioGroup` | `UiListContainer` 的互斥 `UiRadioItem` 集合。`selected_index`、`set_selected_index`、`clear_selection`、`set_on_selection_changed` 与 ButtonGroup 类似；选择一个项目会取消其他项目。 |
| `containers/ui_tab_view.h` — `UiTabView` | 仅显示选中页面的可焦点页面 host。`add_page/extract_page/clear_pages` 管理所有权；`set_selected_index/selected_index`、`set_focused_index/focused_index` 控制显示与导航；`page_at/page_count` 查询借用页面。 |

## 滚动容器

| API | 含义 |
| --- | --- |
| `UiScrollBarVisibility::{Auto,Always,Hidden}` | 自动显示、始终显示或隐藏滚动条。 |
| `UiScrollBarStyle` / `UiScrollBarStyleOverrides` | 滚动轨道、拇指、尺寸与样式覆盖。 |
| `UiScrollContainerStyle` / `UiScrollContainerStyleOverrides` | 视口 chrome 和滚动条的组合样式。 |
| `UiScrollContainer` | 一内容节点的 viewport，同时实现 `UiFocusScope`。`add_child` 被覆写为替换单一内容；优先用 `set_content`。 |
| `set_content/content/clear_content` | 设置、读取或释放唯一滚动内容。`content()` 为不拥有指针。 |
| `set_scroll_axis/scroll_axis/resolved_scroll_axis` | 设置并读取请求的轴及根据内容解析后的轴。 |
| 样式 API | `set_base_style`、`set_style_overrides`、`style`、`style_overrides`、`has_style_overrides`、`clear_style_overrides`；另有 `set_scrollbar_visibility/scrollbar_visibility` 和 `set_scrollbar_style_overrides/scrollbar_style`。 |
| 尺寸与位置 | `content_size`、`set_scroll_offset/scroll_offset`、`set_scroll_offset_x/y`、`scroll_offset_x/y`、`max_scroll_offset`。所有 offset 都会钳制。 |
| 滚动控制 | `set_scroll_step/scroll_step`、各轴 step getter/setter、`scroll_by`、`scroll_to_left/right/top/bottom`、`ensure_visible(target_rect)`。target 为内容局部 rect。 |
| 焦点协议 | `focus_scope_element`、`set_scope_focused`、`is_scope_focused`、`has_focusable_target`、`focus_first_available`、`focused_target`、`can_navigate`、`clear_focus_for_gamepad_scroll`、`restore_focus_after_gamepad_scroll`、`contains_focus_point`；一般由窗口调用。 |

## 复合控件

| API | 含义 |
| --- | --- |
| `composites/ui_labeled_checkbox.h` — `UiLabeledCheckboxLabelPlacement::{Left,Right}` | 标签相对于 checkbox 的侧别。 |
| `UiLabeledCheckboxTextPlacement::{NearBox,FarEdge}` | 标签贴近 indicator 或占据远端。 |
| `UiLabeledCheckboxConfig` | 文本内容、两个 placement 与底层 checkbox config。 |
| `UiLabeledCheckbox` | 将 checkbox 与 label 装配成一个 `UiControl`。构造器可接 config；公开 `set_checkbox_config`、`set_text_content/text_content`、`set_label_placement`、`set_text_placement`、`set_state/state`、`set_checked/is_checked/is_indeterminate`、`toggle`、`set_on_toggled`、`set_enabled/set_focused`、event/render。`checkbox()` 与 `label()` 返回内部借用对象，仅用于查询/有限配置。 |
| `composites/ui_labeled_radio_button.h` | 对应类型为 `UiLabeledRadioLabelPlacement::{Left,Right}`、`UiLabeledRadioTextPlacement::{NearIndicator,FarEdge}`、`UiLabeledRadioButtonConfig` 和 `UiLabeledRadioButton`。其公开文本/placement、`set_radio_button_config`、`set_selected/is_selected`、`set_on_selected`、enable/focus/event/render 语义与带标签 checkbox 对应，并实现 `UiRadioItem`。 |
| `composites/ui_dropdown.h` — `UiDropdownBaseStyle` | trigger 按钮与 popup/list 的基础风格组合。 |
| `UiDropdownOption` | 一个 `UiTextContent`、启用标记和可选样式信息；禁用项不能被选择。 |
| `UiDropdown` | 同时是 `UiControl` 和 `UiTransientPopup`。`set_options/options/add_option/clear_options` 管理选项；`selected_index/set_selected_index` 管理选择；`set_on_selection_changed` 接收新 index。 |
| Dropdown 展开 | `open/close/toggle/is_expanded` 管理列表；`register_as_transient_popup/unregister_as_transient_popup` 绑定/解绑 `UiWindow`。不注册时 trigger 仍可存在，但窗口不会给列表专用的顶层路由和绘制优先级。 |
| Dropdown 样式 | `set_base_style`、style override/query/clear 以及 `set_visual_role/visual_role`；`UiDropdownStyle` 的 `popup_gap`、`option_height`、`popup_max_height` 是 popup 排版参数。 |
| `composites/ui_tab_bar.h` — `UiTabBar` | 由按钮组成的标签列表。`add_tab(content)`；`extract_tab/clear_tabs`；`focused_index/selected_index`、两个 set、`clear_selection`；`set_on_focused_changed/set_on_selected_changed`；`button_at/index_of` 查询。 |
| `composites/ui_tab_container.h` — `UiTabAddResult` | 返回新 tab button 与 page 的借用指针。 |
| `UiTabContainer` | 将 `UiTabBar` 和 `UiTabView` 组合。`add_tab(content,page)` 原子加入一对；`extract_tab/clear_tabs`；`selected_index/set_selected_index`；`tab_bar/tab_view` 读取内部借用部件。 |
| `composites/ui_dialog.h` — `UiDialog` | 阅读型 overlay 复合组件。`set_title_content`、`set_body_content`、`set_action_content` 设置文本；`set_on_action` 设置确认动作；`open/close/is_open` 管理显示；`register_as_overlay/unregister_as_overlay` 绑定窗口。样式 API 使用 `UiDialogStyle` / `UiDialogVisualRole`。 |
| `composites/ui_confirmation_dialog.h` — `UiConfirmationDialogConfig` | title、message、confirm、cancel、close 四个 `UiTextContent`，以及可选 dialog style/overlay options。 |
| `UiConfirmationDialog` | 有确认、取消和关闭的模态友好 overlay。`set_config` 原子更新；各 `set_*_content`/getter 管理文字；`set_on_confirm/set_on_cancel/set_on_close` 注册 callback；`open/close/is_open` 与 overlay 注册 API 语义同 `UiDialog`。 |
| `composites/ui_tooltip.h` — `UiTooltip` | 被动浮层。`bind_trigger/clear_trigger/trigger` 维护借用的触发元素；`set_content` 接收拥有的内容，`release_content` 归还所有权，`clear_content/content` 管理或查询内容；`set_show_delay/show_delay`、`show/hide/is_open` 控制延迟与开关；`register_with_window/unregister_from_window` 管理窗口协作。`update` 跟随触发元素/指针，window 最后提交其命令。 |

## 复合组件的边界

复合组件会把内部元素登记为 composite implementation，以便主题和焦点作为一个整体工作。
调用方不应把从 `checkbox()`、`label()`、`tab_bar()` 等 getter 得到的内部裸指针重新添加到
另一个 host，也不应自行登记为独立 overlay/popup。
