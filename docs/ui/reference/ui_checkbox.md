# UiCheckbox

头文件：`engine/ui/widgets/ui_checkbox.h`。三态控件。

调用：`set_state`/`state` 使用 `Unchecked`、`Checked`、`Indeterminate`；`set_checked`、`is_checked`、`is_indeterminate` 是快捷 API；`toggle` 由当前状态推进。`set_on_toggled` 接收新 `UiCheckboxState`。`set_mark_style`、textures、sounds、config 管理视觉与音效。
