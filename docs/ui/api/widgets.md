# API：控件与视觉元素

本页覆盖 `widgets/` 下的公开类型。所有带 `style` / `style_overrides` / `visual_role`
的类型遵循相同规则：base style 是完整基线，overrides 只覆盖指定字段，role 是主题选择键；
`clear_style_overrides` 回到无局部覆盖状态。所有 `SDL_Texture*` 都是借用指针，调用方负责
纹理寿命。

## 按钮、选择与调节

| API | 含义 |
| --- | --- |
| `widgets/ui_button.h` — `UiButtonTextures` | idle/focused/pushed/disabled 四个可选状态纹理。 |
| `UiButtonIconContent` / `UiButtonTextureSetContent` / `UiButtonContent` | 纯图标、完整纹理集，以及空/文本/图标/纹理集的 variant 内容。 |
| `UiButtonSounds` | focus、press、click 声音 key。 |
| `UiButtonStyle` / `UiButtonStyleOverrides` / `UiButtonConfig` | chrome+文字样式、其稀疏覆盖，以及构造/原子更新所需 content/sounds/覆盖。 |
| `UiButton::UiButtonVisualMode` | `None`、`Text`、`Textured`、`Icon`；`visual_mode()` 报告当前内容模式。 |
| `UiButton` 构造器 | 接受 rect、position/size 或中心标签；带 config 的重载同时应用配置。 |
| `reset/set_enabled/set_focused/on_ui_input_event/submit_ui_render_commands` | 复位、交互状态、事件和绘制入口。Confirm 或主指针点击会触发 callback（若可交互）。 |
| `set_button_config` | 原子替换 content、声音和样式覆盖，避免中间视觉状态。 |
| `set_text_content/text_content` | 设置/读取 `UiTextContent`。 |
| `set_state_textures/clear_state_textures/has_state_textures` | 配置/移除状态纹理并查询是否存在。 |
| `set_sounds/clear_sounds/sounds`、`set_on_click` | 配置声音或 click 回调。 |
| `set_base_style` 到 `clear_style_overrides` | 管理按钮样式；`set_visual_role/visual_role` 选择 `UiButtonVisualRole`；`set_typography_role/typography_role` 选择字体角色。 |
| `set_padding/padding` | 文本或图标的内部留白，默认 10。 |
| `widgets/ui_checkbox.h` — `UiCheckboxState` | `Unchecked`、`Checked`、`Indeterminate` 三态。 |
| `UiCheckboxMarkStyle` | `Checkmark`、`FilledBox` 或 `RadioDot` 的程序绘制标记。 |
| `UiCheckboxVisualStateTextures` / `UiCheckboxTextures` | 一个交互状态的纹理组 / 覆盖三个逻辑状态的完整纹理组。 |
| `UiCheckboxSounds`、`UiCheckboxStyle`、`UiCheckboxStyleOverrides`、`UiCheckboxConfig` | 声音、chrome+mark 样式、稀疏覆盖和构造配置。 |
| `UiCheckbox` 构造器与基础生命周期 API | 与 `UiButton` 的三种几何构造和可选 config 相同；支持 reset/enable/focus/event/render。 |
| `set_checkbox_config` | 原子更新纹理、声音和样式覆盖。 |
| `set_state/state`、`set_checked/is_checked/is_indeterminate`、`toggle` | 直接设置三态、布尔快捷方式和按支持序列推进状态。直接 set 不等同于用户交互。 |
| `set_state_textures/clear_state_textures/state_textures/has_complete_state_textures` | 管理并查询纹理皮肤完整性。 |
| `set_sounds/clear_sounds/sounds/set_on_toggled` | 管理声音和状态变更回调（参数为新的 `UiCheckboxState`）。 |
| `set_mark_style/mark_style`、`set_padding/padding` | 选择标记样式和内部留白。其余样式 API 与按钮一致。 |
| `widgets/ui_radio_button.h` — `UiRadioButtonSounds` | focus/press/select 声音 key。 |
| `UiRadioButtonStyle` / `Overrides` / `Config` | chrome+mark 样式、覆盖；config 含 sounds、覆盖与 padding。 |
| `UiRadioButton` | 实现 `UiRadioItem` 的最终单选控件。构造、reset、enable、focus、event、render 与 checkbox 同类。`set_radio_button_config` 原子应用 config；`set_selected/is_selected` 设置或读取选中；`set_on_selected` 在选中时调用；样式和 padding API 与 checkbox 对应。单选互斥由 `UiRadioGroup` 负责。 |
| `widgets/ui_slider.h` — `UiSliderOrientation` | `Horizontal` 或 `Vertical`。 |
| `UiSliderValueDisplay` | `None`、`Value`、`Percent`，控制内部显示值。 |
| `UiSliderSounds`、`UiSliderStyle`、`Overrides`、`Config` | focus/press/change 声音、样式以及构造配置。 |
| `UiSlider` | 构造、reset、enable、focus、event、render；`set_slider_config` 原子应用配置。 |
| `set_range/min_value/max_value`、`set_value/value`、`set_step/step` | 设置值域、钳制的当前值和键盘/手柄步长。 |
| `set_orientation/orientation`、`set_value_display/value_display` | 设置主轴和内部数值显示方式。 |
| `set_on_value_changed` | 值实际改变时接收新 `float`。`set_state_textures`、声音、样式、字体角色与 padding API 的语义同其他交互控件。 |
| `widgets/ui_drag_handle.h` — `UiDragAxis::{Horizontal,Vertical,Both}` | 限制拖拽可改变的坐标轴。 |
| `UiDragHandleTextures`、`UiDragHandleStyle`、`Overrides`、`Config` | 状态纹理、chrome 样式及构造配置。 |
| `UiDragHandle` | 可拖拽的 `UiControl`；构造/生命周期、config、纹理、样式、padding 与其他控件一致。`set_drag_axis/drag_axis` 限制移动；`set_drag_bounds/clear_drag_bounds/drag_bounds` 设置可选约束矩形；`set_on_dragged` 接收新位置；`set_on_drag_started/ended` 接收拖拽边界通知。 |
| `widgets/ui_bar.h` — `BarFillDirection` | `LeftToRight`、`RightToLeft`、`TopToBottom`、`BottomToTop`。 |
| `UiBar` | 非交互进度条。`set_range`、`set_value` 通过数值域得出 ratio；`set_ratio` 直接使用归一化填充；对应 getter 读取结果。样式/role API 使用 `UiBarStyle` 与 `UiBarVisualRole`；`set_fill_direction`、`set_padding` 控制填充方向和视觉内边距。 |

## 文本、数字与图像

| API | 含义 |
| --- | --- |
| `widgets/ui_text_input.h` — `UiTextInputStyle` / `Overrides` | chrome、text、placeholder、caret 的完整/稀疏样式。 |
| `UiTextInput` | 单行 UTF-8/IME 输入控件。构造、reset、enable、focus、event、render 是其基础入口；焦点获取时声明文本输入所有权，失焦/销毁释放。 |
| `set_text/text/clear_text` | 替换、读取或清空 buffer，并维护 caret/composition。 |
| `set_placeholder_content/placeholder_content` | buffer 为空时显示的本地化 key 或原样提示文字。 |
| `set_on_text_changed/set_on_submit` | 文字实际变更时或确认提交时接收 `string_view`。回调不拥有该 view。 |
| `set_max_length/max_length` | 设置可选长度限制，`nullopt` 表示不限制。 |
| 样式、`set_typography_role`、`set_placeholder_typography_role`、`set_padding` | 管理视觉和文字角色；默认文字角色为 Input，placeholder 为 InputPlaceholder。 |
| `widgets/label/ui_label.h` — `UiLabel` | 单行文本。构造器可直接带 `UiTextContent`；`set_text_content/text_content` 管理内容；style/role API 使用 `UiLabelStyle`/`UiLabelVisualRole`。 |
| `set_horizontal_align` / `set_vertical_align` | 设置 `TextHorizontalAlign`/`TextVerticalAlign`；`set_target_height/target_height/clear_target_height` 设置可选输出高度并保持比例；`set_typography_role` 和 `set_padding` 管理字体与留白。 |
| `UiBlinkLabel`、`UiFadeLabel`、`UiPulseLabel` | 分别继承 `UiLabel` 并实现 `Updatable`；构造参数与 Label 相同，另外提供 `configure_blink/fade/pulse`、`play_*`、`is_*_finished` 等与对应 opacity core 同名的播放控制，`update` 将 core opacity 写回元素。 |
| `widgets/text/ui_text_block.h` — `UiTextBlock` | 多行文字，`content_extent` 基于解析和换行结果测量。`set_text_content/text_content/clear_text` 管理内容；style/role/typography/padding/horizontal align API 与 Label 对应，使用 `UiTextBlockStyle` 和 `UiTextBlockVisualRole`。 |
| `widgets/number/ui_number.h` — `UiNumberSuffix::{None,Percent}` | 数字后缀。 |
| `UiNumber` | 基于共享数字缓存的数值元素。`set_value/value` 管理数值；样式、对齐、字体和 padding API 与 Label 类似。 |
| `set_digit_spacing`、`set_fixed_glyph_advance/clear_fixed_glyph_advance` | 控制字距和可选固定字宽。 |
| `set_target_height/clear_target_height`、`set_decimal_places`、`set_trim_trailing_zeros`、`set_keep_decimal_point`、`set_suffix` | 控制缩放、格式精度、零裁剪、小数点与 percent 后缀；每组都有同名 getter。 |
| `widgets/image/ui_image.h` — `UiImage` | 以借用纹理绘制一个图像。四个构造器支持位置、rect、中心和“source size + render size”。`set_texture/texture` 替换/读取纹理；`set_source_rect/clear_source_rect` 启用/取消纹理空间裁剪。 |
| `UiBlinkImage`、`UiFadeImage`、`UiPulseImage` | `UiImage` 的透明度动画版本；保留 Image 的纹理 API，增加与效果 core 对应的配置、播放、完成查询和 `update`。 |
| `widgets/image/ui_animation.h` — `UiAnimation` | 通过动画 key 展示注册帧动画。`set_animation_key` 成功绑定并从首帧播放，未注册 key 返回 false；`animation_key` 读取 key。 |
| `set_loop/is_looping`、`play/pause/resume/reset`、`is_finished/is_paused` | 本控件的循环覆盖与播放控制；`update` 推进动画，`submit_ui_render_commands` 绘制当前帧。 |

## 继承 API 的使用边界

本页的所有 widget 都继承 `UiElement`，交互 widget 还继承 `UiControl`；因此均可使用
`set_screen_rect`、`set_order`、`set_opacity` 等[核心 API](core-layout-input.md)。不要调用其
protected 命中测试、绘制颜色或内部状态辅助函数；它们不是稳定公开契约。
