# MoonLine UI API 参考

本目录以 `engine/ui/**/*.h` 为唯一事实来源。每个公开类都有独立参考页；跨组件的重复调用只在专题页解释一次。

## 入口

- [公开头文件覆盖清单](coverage.md)
- [公共元素、生命周期与几何](concepts/element-lifecycle.md)
- [子项所有权与布局](concepts/child-layout.md)
- [输入、焦点与滚动](concepts/input-focus-scroll.md)
- [样式、主题与视觉角色](concepts/style-theme.md)
- [文本、数字与排版](concepts/text-number.md)
- [窗口表面与注册](concepts/window-surfaces.md)
- [动画与效果](concepts/effects.md)

## 独立组件参考

- Core 与运行时：[UiElement](reference/ui-element.md)、[UiChildHost](reference/ui-child-host.md)、[UiControl](reference/ui-control.md)、[UiControlFocusScopeHost](reference/ui-control-focus-scope-host.md)、[UiWindow](reference/ui-window.md)
- Widgets：[Button](reference/ui-button.md)、[Checkbox](reference/ui-checkbox.md)、[RadioButton](reference/ui-radio-button.md)、[Slider](reference/ui-slider.md)、[TextInput](reference/ui-text-input.md)、[DragHandle](reference/ui-drag-handle.md)、[Bar](reference/ui-bar.md)、[Label](reference/ui-label.md)、[TextBlock](reference/ui-text-block.md)、[Number](reference/ui-number.md)、[Image](reference/ui-image.md)、[Animation](reference/ui-animation.md)
- Containers：[List](reference/ui-list-container.md)、[Grid](reference/ui-grid-container.md)、[Panel](reference/ui-panel.md)、[ScrollContainer](reference/ui-scroll-container.md)、[ChromeContainer](reference/ui-chrome-container.md)、[ButtonGroup](reference/ui-button-group.md)、[RadioGroup](reference/ui-radio-group.md)、[TabView](reference/ui-tab-view.md)
- Composites：[Dropdown](reference/ui-dropdown.md)、[Dialog](reference/ui-dialog.md)、[ConfirmationDialog](reference/ui-confirmation-dialog.md)、[Tooltip](reference/ui-tooltip.md)、[TabBar](reference/ui-tab-bar.md)、[TabContainer](reference/ui-tab-container.md)、[LabeledCheckbox](reference/ui-labeled-checkbox.md)、[LabeledRadioButton](reference/ui-labeled-radio-button.md)

组件页只描述特有接口；继承的几何、样式、所有权、输入和窗口注册规则请使用上述专题页。
