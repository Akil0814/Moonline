# UiConfirmationDialog

头文件：`engine/ui/composites/ui_confirmation_dialog.h`。固定 Cancel/Confirm 结构的 overlay。

调用：通过 `UiConfirmationDialogConfig` 和 `set_config` 提供 title/message/button 文本与角色；`set_on_confirm` 注册确认回调；注册、打开、关闭流程与 `UiDialog` 相同。确认动作会先关闭 dialog 再调用回调，回调可安全切换场景。
