# 公开头文件覆盖清单

命名规则：类参考页严格使用对应头文件 stem，例如 `ui_scroll_container.h` → `reference/ui_scroll_container.md`；专题页使用 snake_case。以下映射覆盖 `engine/ui` 所有公开头文件。

| 头文件组 | 文档 |
| --- | --- |
| `core/ui_element.h` | [UiElement](reference/ui_element.md) |
| `core/ui_child_host.h` | [UiChildHost](reference/ui_child_host.md)、[child_layout](concepts/child_layout.md) |
| `core/ui_control.h`, `ui_focusable.h`, `ui_radio_item.h`, `ui_text_align.h`, `ui_render_command_range_utils.h` | [UiControl](reference/ui_control.md)、[input_focus_scroll](concepts/input_focus_scroll.md)、[text_number](concepts/text_number.md) |
| `layout/*.h` | [child_layout](concepts/child_layout.md) |
| `input/*.h`, `input/contracts/*.h` | [input_focus_scroll](concepts/input_focus_scroll.md)、[UiInputRouter](reference/ui_input_router.md)、[UiInputState](reference/ui_input_state.md)、[UiGamepadScrollSynthesizer](reference/ui_gamepad_scroll_synthesizer.md) |
| `focus/*.h` | [UiFocusScope](reference/ui_focus_scope.md)、[UiControlFocusScopeHost](reference/ui_control_focus_scope_host.md)、[input_focus_scroll](concepts/input_focus_scroll.md) |
| `scroll/ui_scroll_state.h` | [UiScrollState](reference/ui_scroll_state.md) |
| `effects/*.h` | [effects](concepts/effects.md)、[UiTranslationAnimationPlayer](reference/ui_translation_animation_player.md)、各 opacity core 参考页 |
| `number/ui_digit_renderer.h` | [UiDigitRenderer](reference/ui_digit_renderer.md) |
| `text/*.h` | [text_number](concepts/text_number.md) |
| `style/*.h` | [style_theme](concepts/style_theme.md)、[UiStyleState](reference/ui_style_state.md)、[UiThemeStyleResolver](reference/ui_theme_style_resolver.md)、[UiThemeManager](reference/ui_theme_manager.md) |
| `widgets/**/*.h` | 对应 `reference/ui_*.md` widget 页面；文本、图像变体与效果页也独立存在 |
| `containers/*.h` | 对应 `reference/ui_*_container.md`、group、tab view 页面 |
| `composites/*.h` | 对应 `reference/ui_*.md` composite 页面 |
| `window/*.h` | [window_surfaces](concepts/window_surfaces.md)、[UiWindow](reference/ui_window.md) |

新增公开头文件时必须在本表增加映射；新增公开类必须新建与头文件 stem 一致的参考页。
