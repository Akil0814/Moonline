# MoonLine UI API 参考

本目录以 `engine/ui/**/*.h` 为唯一事实来源。每个公开类都有独立参考页；跨组件的重复调用只在专题页解释一次。

## 入口

- [公开头文件覆盖清单](coverage.md)
- [公共元素、生命周期与几何](concepts/element_lifecycle.md)
- [子项所有权与布局](concepts/child_layout.md)
- [输入、焦点与滚动](concepts/input_focus_scroll.md)
- [样式、主题与视觉角色](concepts/style_theme.md)
- [文本、数字与排版](concepts/text_number.md)
- [窗口表面与注册](concepts/window_surfaces.md)
- [动画与效果](concepts/effects.md)
- [公开 API 调用模式](concepts/call_patterns.md)

## 独立组件参考

- Core 与运行时：[UiElement](reference/ui_element.md)、[UiChildHost](reference/ui_child_host.md)、[UiControl](reference/ui_control.md)、[UiControlFocusScopeHost](reference/ui_control_focus_scope_host.md)、[UiWindow](reference/ui_window.md)
- Widgets：[Button](reference/ui_button.md)、[Checkbox](reference/ui_checkbox.md)、[RadioButton](reference/ui_radio_button.md)、[Slider](reference/ui_slider.md)、[TextInput](reference/ui_text_input.md)、[DragHandle](reference/ui_drag_handle.md)、[Bar](reference/ui_bar.md)、[Label](reference/ui_label.md)、[TextBlock](reference/ui_text_block.md)、[Number](reference/ui_number.md)、[Image](reference/ui_image.md)、[Animation](reference/ui_animation.md)
- Containers：[List](reference/ui_list_container.md)、[Grid](reference/ui_grid_container.md)、[Panel](reference/ui_panel.md)、[ScrollContainer](reference/ui_scroll_container.md)、[ChromeContainer](reference/ui_chrome_container.md)、[ButtonGroup](reference/ui_button_group.md)、[RadioGroup](reference/ui_radio_group.md)、[TabView](reference/ui_tab_view.md)
- Composites：[Dropdown](reference/ui_dropdown.md)、[Dialog](reference/ui_dialog.md)、[ConfirmationDialog](reference/ui_confirmation_dialog.md)、[Tooltip](reference/ui_tooltip.md)、[TabBar](reference/ui_tab_bar.md)、[TabContainer](reference/ui_tab_container.md)、[LabeledCheckbox](reference/ui_labeled_checkbox.md)、[LabeledRadioButton](reference/ui_labeled_radio_button.md)

组件页只描述特有接口；继承的几何、样式、所有权、输入和窗口注册规则请使用上述专题页。
