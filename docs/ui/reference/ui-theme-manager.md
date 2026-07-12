# UiThemeManager

头文件：`engine/ui/style/ui_theme_manager.h`。主题注册与应用管理器。

调用：`register_root(UiChildHost&)` 返回 move-only `UiThemeRegistration`；保存 registration 即维持绑定。`set_theme` 切换内置主题，`current_builtin_theme`/`current_theme` 查询，`reapply_theme` 强制重算，`detach_subtree`/`detach_all` 注销。
