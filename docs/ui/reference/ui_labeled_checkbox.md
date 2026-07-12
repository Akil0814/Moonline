# UiLabeledCheckbox

头文件：`engine/ui/composites/ui_labeled_checkbox.h`。组合 Checkbox 与 Label。

调用：使用 `UiLabeledCheckboxConfig` 或 `set_labeled_checkbox_config` 设置文本、位置和样式；状态 API 为 `set_state`、`set_checked`、`toggle`、`is_checked`；`set_on_toggled` 注册回调。`set_label_placement`、`set_text_placement`、`set_label_spacing` 调整文本布局。
