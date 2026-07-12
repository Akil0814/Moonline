# UiLabeledRadioButton

头文件：`engine/ui/composites/ui_labeled_radio_button.h`。组合 RadioButton 与 Label，并实现 `UiRadioItem`。

调用：使用 config 或 `set_labeled_radio_button_config`；`set_selected`/`is_selected` 与 `set_on_selection_changed` 管理选择；`set_label_placement`、`set_text_placement`、`set_label_spacing`、`set_typography_role` 调整文本。放入 `UiRadioGroup` 管理互斥。
