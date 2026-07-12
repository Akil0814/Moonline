# 公开头文件覆盖清单

下表是 `engine/ui/**/*.h` 的文档映射。类名页面独立；只含枚举、结构体、自由函数或内部协作契约的头文件进入专题。

| 头文件组 | 文档 |
| --- | --- |
| `core/ui_element.h` | [UiElement](reference/ui-element.md) |
| `core/ui_child_host.h` | [UiChildHost](reference/ui-child-host.md) + [子项布局](concepts/child-layout.md) |
| `core/ui_control.h`, `core/ui_focusable.h` | [UiControl](reference/ui-control.md) + [输入焦点](concepts/input-focus-scroll.md) |
| `core/ui_radio_item.h`, `core/ui_text_align.h`, `core/ui_render_command_range_utils.h` | [输入焦点](concepts/input-focus-scroll.md)、[文本数字](concepts/text-number.md)、[元素专题](concepts/element-lifecycle.md) |
| `layout/*.h` | [子项布局](concepts/child-layout.md) |
| `input/ui_input_types.h`, `ui_input_frame.h`, `ui_input_router.h`, `ui_input_state.h`, `ui_gamepad_scroll_synthesizer.h`, `input/contracts/*.h` | [输入焦点](concepts/input-focus-scroll.md)、[InputRouter](reference/ui-input-router.md)、[InputState](reference/ui-input-state.md)、[Synthesizer](reference/ui-gamepad-scroll-synthesizer.md) |
| `focus/ui_focus_scope.h`, `ui_control_focus_scope_host.h`, `ui_delegated_focus_mixin.h`, `ui_focus_scope_utils.h` | [UiFocusScope](reference/ui-focus-scope.md)、[UiControlFocusScopeHost](reference/ui-control-focus-scope-host.md)、[输入焦点](concepts/input-focus-scroll.md) |
| `scroll/ui_scroll_state.h` | [UiScrollState](reference/ui-scroll-state.md) |
| `effects/ui_translation_animation_player.h` | [UiTranslationAnimationPlayer](reference/ui-translation-animation-player.md) |
| `effects/ui_opacity_common.h`, `ui_opacity_blink_core.h`, `ui_opacity_fade_core.h`, `ui_opacity_pulse_core.h` | [动画效果](concepts/effects.md)、[BlinkCore](reference/ui-opacity-blink-core.md)、[FadeCore](reference/ui-opacity-fade-core.md)、[PulseCore](reference/ui-opacity-pulse-core.md) |
| `number/ui_digit_renderer.h` | [UiDigitRenderer](reference/ui-digit-renderer.md) |
| `text/ui_text_content.h`, `text/ui_typography.h` | [文本数字](concepts/text-number.md) |
| `style/ui_interaction_style.h`, `ui_palette.h`, `ui_style_defaults.h`, `ui_visual_roles.h`, `ui_visual_styles.h`, `ui_theme.h` | [样式主题](concepts/style-theme.md) |
| `style/ui_style.h`, `ui_theme_style_resolver.h`, `ui_theme_manager.h` | [UiStyleState](reference/ui-style-state.md)、[Resolver](reference/ui-theme-style-resolver.md)、[UiThemeManager](reference/ui-theme-manager.md) |
| `widgets/ui_button.h`, `ui_checkbox.h`, `ui_radio_button.h`, `ui_slider.h`, `ui_drag_handle.h`, `ui_text_input.h`, `ui_bar.h` | [Button](reference/ui-button.md)、[Checkbox](reference/ui-checkbox.md)、[Radio](reference/ui-radio-button.md)、[Slider](reference/ui-slider.md)、[DragHandle](reference/ui-drag-handle.md)、[TextInput](reference/ui-text-input.md)、[Bar](reference/ui-bar.md) |
| `widgets/label/ui_label.h`, `ui_fade_label.h`, `ui_blink_label.h`, `ui_pulse_label.h` | [Label](reference/ui-label.md)、[FadeLabel](reference/ui-fade-label.md)、[BlinkLabel](reference/ui-blink-label.md)、[PulseLabel](reference/ui-pulse-label.md) |
| `widgets/text/ui_text_block.h`, `widgets/number/ui_number.h` | [TextBlock](reference/ui-text-block.md)、[Number](reference/ui-number.md) |
| `widgets/image/ui_image.h`, `ui_animation.h`, `ui_fade_image.h`, `ui_blink_image.h`, `ui_pulse_image.h` | [Image](reference/ui-image.md)、[Animation](reference/ui-animation.md)、[FadeImage](reference/ui-fade-image.md)、[BlinkImage](reference/ui-blink-image.md)、[PulseImage](reference/ui-pulse-image.md) |
| `containers/ui_list_container.h`, `ui_grid_container.h`, `ui_panel.h`, `ui_scroll_container.h`, `ui_chrome_container.h`, `ui_button_group.h`, `ui_radio_group.h`, `ui_tab_view.h` | 对应每个 [container 参考页](README.md#独立组件参考) |
| `composites/ui_dropdown.h`, `ui_dialog.h`, `ui_confirmation_dialog.h`, `ui_tooltip.h`, `ui_tab_bar.h`, `ui_tab_container.h`, `ui_labeled_checkbox.h`, `ui_labeled_radio_button.h` | 对应每个 [composite 参考页](README.md#独立组件参考) |
| `window/ui_overlay.h`, `ui_transient_popup.h`, `ui_window.h` | [窗口表面](concepts/window-surfaces.md)、[UiWindow](reference/ui-window.md) |

维护规则：新增公开头文件必须在本表增加一行，并创建独立类页或补充其所属专题。
