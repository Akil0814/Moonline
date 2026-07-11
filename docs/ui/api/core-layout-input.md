# API：核心、布局、输入与基础设施

本页覆盖 `core/`、`layout/`、`input/`、`focus/`、`scroll/`、`text/`、`effects/` 与
`number/` 的公开声明。除非另有说明，几何参数为屏幕空间 `Rect`/`Vector2`。

## 核心对象与所有权

| 头文件 / API | 含义 |
| --- | --- |
| `core/ui_element.h` — `UiFromCenterTag`、`from_center` | 选择以中心点解释构造位置的标签；普通构造器以左上角解释位置。 |
| `UiElement(Rect/position+size/center+size, order)` | 所有 UI 元素的基类；`order` 越大越靠上并优先输入。不可复制或移动。 |
| `submit_ui_render_commands(out)` | 虚函数，追加本元素绘制命令；默认不输出。 |
| `content_extent()` | 供父布局测量的内在尺寸；默认等于 `size()`。 |
| `reset()` | 复位 SceneObject、布局父指针和 opacity；复用对象时调用。 |
| `set_screen_rect/set_position/set_center/set_size` | 更新最终几何；尺寸变化会通知布局父节点。 |
| `screen_rect/position/center/size/layout_parent` | 读取当前几何与非拥有布局父节点。 |
| `set_order/order`、`set_opacity/opacity` | 设置/读取层级与 0–255 透明度。 |
| `update_when_paused/receive_input_when_paused` | UI 固定返回 `true`，使暂停场景仍可展示和交互。 |
| `core/ui_control.h` — `UiControl` | 可聚焦的 `UiElement`。`set_enabled/is_enabled` 控制是否可交互；`set_focused/is_focused` 由焦点系统设置并供控件刷新视觉。 |
| `core/ui_focusable.h` — `UiFocusable` | 自定义可焦点对象应实现的最小协议：`set_focused` 与 `is_focused`。 |
| `core/ui_radio_item.h` — `UiRadioItem` | 可被 `UiRadioGroup` 选择的协议：`set_selected`、`is_selected`、`set_on_selected`。 |
| `core/ui_text_align.h` — `UiTextAlign::{Left,Center,Right}` | 文本在其布局矩形内的水平对齐。 |
| `core/ui_render_command_range_utils.h` | `apply_ui_opacity_to_render_command_range`、`clip_ui_render_command_range` 处理一段命令；`clip_ui_render_command` 裁剪单条。供 host/自定义容器统一裁剪与透明度，参数 `begin` 是命令起始下标。 |

### `UiChildHost`

`core/ui_child_host.h` 的 `UiChildHost` 是拥有子元素的基础容器，并同时是 `Updatable`、
`UiInputFrameReceiver` 与 `UiInputEventReceiver`。

| API | 含义 |
| --- | --- |
| `ChildEntry` | 一个拥有的 `element`、其 `UiLayoutChildOptions`，以及主题关系元数据。外部调用不应修改其所有权。 |
| `UiChildStyleRelation::{Independent,CompositeImplementation}` | 独立 child 使用自己的主题语义；复合实现 child 由复合组件的 style owner 驱动。 |
| `add_child(unique_ptr, options)` / `insert_child(..., index, options)` | 接收所有权并返回非拥有指针；插入失败返回空。后者供派生容器控制顺序。 |
| `create_child<T>(args...)` / `create_child<T>(options,args...)` | 原地创建并收养 `UiElement` 派生类；与 `add_child` 一样返回借用指针。 |
| `clear_children()` | 释放全部 child 并解除布局父关系。 |
| `extract_child(index)` | 脱离指定 child 并归还 `unique_ptr`；索引无效时返回空。 |
| `child_count/child_at` | 查询 child；`child_at` 可能为 null，返回指针不拥有对象。 |
| `set_child_layout_options/child_layout_options/move_child` | 修改单项布局、读取布局或重排 child。移动成功返回 `true`。 |
| `set_padding/padding`、`set_clip_children/clips_children` | 设置内容内边距和子命令裁剪策略。 |
| `mark_layout_dirty/update_layout_if_dirty` | 标记或按需重建布局；一般不需要手工调用，尺寸/child 改动会触发。 |
| `update/on_ui_input_frame/on_ui_input_event/submit_ui_render_commands` | 向存活且可见 child 分发更新、帧状态、事件和渲染。事件遇到已消费 child 即停止。 |

## 布局

`layout/ui_layout_types.h`：`UiLayoutAnchor` 的九个值为 Top/Center/Bottom × Left/Center/Right；
`UiLayoutDirection::{Horizontal,Vertical}` 决定堆叠主轴；`UiLayoutAlign::{Start,Center,End}`
决定横轴对齐。`UiLayoutPadding` 与 `UiLayoutMargin` 均为 left/top/right/bottom 浮点 inset。
`UiLayoutChildOptions` 包含 `_anchor`、`_margin`、`_cross_align`、`_size_override` 和三个启用
标志（自定义横轴对齐、填满横轴、使用尺寸覆盖）。`UiLayoutTransform` 是 translation/scale
元数据，当前供需要变换信息的布局使用。

| 头文件 / API | 含义 |
| --- | --- |
| `layout/ui_layout_geometry.h` — `clamp_non_negative`、`clamp_size` | 将负长度或尺寸钳制到零。 |
| `padded_content_rect(rect,padding)` | 从 host 矩形扣除内边距后的可布局区域。 |
| `anchored_rect`、`aligned_rect_in_bounds` | 按 anchor 与 margin 将给定尺寸放入 bounds；后者为布局共用的对齐版本。 |
| `layout/ui_anchor_layout.h` — `layout_anchored_children(children,bounds)` | 对每个 child 应用其 anchor/margin 选项。 |
| `layout/ui_list_layout.h` — `UiListLayoutConfig` | `direction=Vertical`、`cross_align=Center`、`item_spacing=16` 的列表配置。 |
| `layout_list_children` / `intrinsic_list_extent` | 排布列表 / 计算含 padding 的最小内容范围。 |
| `layout/ui_grid_layout.h` — `UiGridLayoutConfig` | `column_count=4`、`cell_spacing=(12,12)`、`cell_anchor=Center`、`fill_by_row=true`。 |
| `layout_grid_children` / `intrinsic_grid_extent` | 排布统一网格单元 / 测量网格内在尺寸。 |

## 输入与焦点

| API | 含义 |
| --- | --- |
| `input/ui_input_types.h` — `UiAction` | `NavigateLeft/Right/Up/Down`、`Confirm`、`Cancel`、`Tab`、`Backspace`、`DeleteKey`、`Home`、`End`；`None` 和 `Count` 不是可跟踪动作。 |
| `UiInputEventType` | `ActionPressed/Released`、鼠标移动/按下/释放、滚轮、文本输入/编辑、轴变化；决定 `UiInputEvent` 哪些字段有效。 |
| `UiInputEvent` | 归一化事件；含 action、device、raw control/axis、鼠标/滚轮坐标、输入法 composition、axis_value 与 text。未使用字段保持默认值。 |
| `UiInputState::set/is_pressed/is_just_pressed/is_just_released` | 设置或查询一个动作的持续/本帧边沿状态；`None`/`Count` 查询恒为 false。 |
| `UiInputFrame` | 一帧的 `state`、`active_device` 和 `device_switched_this_frame`。 |
| `UiInputRouter::route_frame/route_event` | 从原始帧建立 held-state；从一个原始事件生成零到多个 UI 事件。 |
| `UiInputRouter::synthesize_events/reset_transient_state` | 从手柄轴合成滚轮事件；在换屏或交互会话结束时清除内部状态。 |
| `UiGamepadScrollSynthesizer::synthesize/reset` | 单独使用的手柄轴→滚轮合成器；`synthesize` 无可发送事件时返回 `nullopt`。 |
| `contracts/UiInputFrameReceiver` | 实现 `on_ui_input_frame(const UiInputFrame&)` 以接收每帧状态。 |
| `contracts/UiInputEventReceiver` | 实现 `on_ui_input_event(const UiInputEvent&)`；返回 `true` 表示事件已消费。 |
| `focus/ui_focus_scope.h` — `UiFocusNeighbors` | 某控件的 up/down/left/right 可选邻居。 |
| `UiFocusScopeNeighbors` | 某 scope 的四向相邻 scope，空指针表示边界。 |
| `UiFocusScope` | scope 协议：提供根元素、scope 焦点状态、可用目标、首焦点、当前目标、导航能力、手柄滚动焦点抑制/恢复和指针命中。 |
| `UiControlFocusScopeHost` | `UiChildHost` + `UiFocusScope` 实现。`set_focused_target`、`focused_target`、`focus_first_available`、`has_focusable_target`、`can_navigate` 管理内部焦点；`FocusEntry` 将 `UiControl*` 与方向邻居绑定。 |
| `UiDelegatedFocusMixin` 与 `focus_scope_utils` | 供复合/嵌套容器把焦点委托到内部区域；`collect_focus_scopes`、`collect_focused_scroll_containers`、`find_focus_scope_at` 是遍历辅助函数。 |

## 滚动、文本、数值与效果

| API | 含义 |
| --- | --- |
| `UiScrollAxis::{Auto,Vertical,Horizontal,Both}` | Auto 根据 viewport/content 解析可滚动轴；其余强制允许轴。 |
| `UiScrollState` | 与控件无关的滚动模型。`reset`；axis、viewport/content、offset、step 的 set/get；`effective_content_size`、`resolved_axis`、`max_offset`、`can_scroll_*`；水平/垂直 ratio 读写；`scroll_by`、`scroll_to_left/right/top/bottom` 和 `ensure_visible(local_rect)` 均会钳制。 |
| `text/ui_text_content.h` — `UiTextContentKind::{None,TextKey,RawText}` | 区分空内容、本地化 key 与原样字符串。 |
| `UiTextContent::empty` | 无 kind 或空 value 时为 true。 |
| `ui_text_key(value)` / `ui_raw_text(value)` | 创建文本来源；空 value 返回空内容。 |
| `text/ui_typography.h` — `UiTypographyRole` | `Label`、`LabelMuted`、`Title`、`Subtitle`、`Button`、`ButtonCompact`、`Input`、`InputPlaceholder`、`Number`、`DialogTitle`、`DialogBody`、`DialogAction`、`SliderValue`、`CheckboxLabel`、`RadioLabel`。 |
| `UiResolvedTextStyle`、`kLoadedUiFontPointSizes`、`resolve_loaded_ui_font_point_size`、`resolve_ui_typography` | 已解析的 point size、是否允许换行和默认水平对齐；已加载字号清单；把请求字号映射到最近可用字号；把语义 role 映射为解析结果。 |
| `number/ui_digit_renderer.h` — `UiDigitRenderRequest` | 数值、suffix、目标 rect、颜色、对齐和 digit cache 的绘制请求。 |
| `UiDigitRenderer::set_digit_cache/digit_cache/submit` | 设置/读取非拥有数字纹理缓存，并把请求转换成 UI render commands。 |
| `effects/ui_opacity_common.h` | `clamp_unit`、`ratio`、`ease_in_out`、`lerp_opacity`：透明度效果的纯函数工具。 |
| `UiOpacityBlinkCore` | `reset`、`configure_playback(mode,hold,visible,hidden,cycles)`、`play`、`update`、`opacity`、`is_finished`；blink mode 为 `VisibleFirst`/`HiddenFirst`，有限 cycle 为 0 时立即完成。 |
| `UiOpacityFadeCore` | 同样的 reset/configure/play/update/query；mode 为 `FadeIn`、`FadeOut`、`FadeInOut`。 |
| `UiOpacityPulseCore` | 同样的 reset/configure/play/update/query；mode 为 `MinToMax`/`MaxToMin`；配置包含 cycle 与 min/max opacity，范围倒置时自动交换。 |
