# UiRadioButton

头文件：`engine/ui/widgets/ui_radio_button.h`。实现 `UiRadioItem` 的单选控件。

调用：`set_selected`/`is_selected` 设置或查询；`select` 请求选择并触发 `set_on_selection_changed` 回调。单个 RadioButton 不管理同级互斥；放入 `UiRadioGroup` 获得组行为。
