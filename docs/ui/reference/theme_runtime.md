# 样式与主题运行时

覆盖头文件：`ui_style.h`、`ui_theme_style_resolver.h`、`ui_theme_manager.h`。

- `UiStyleState<Style>`：组件内部合成 base style 与 sparse overrides；调用 `reset`、`set_base_style`、`set_style_overrides`、`effective_style`、`clear_style_overrides`。
- `UiThemeStyleResolver`：注册 adapter 后使用 `apply` 或 `apply_subtree`，将 `UiTheme` 映射为具体 element base style。
- `UiThemeManager`：应用代码入口。保存 `register_root` 返回的 `UiThemeRegistration`；使用 `set_theme`、`current_theme`、`reapply_theme`，或用 `detach_subtree`/`detach_all` 解除绑定。

完整 override 替换规则见 [style_theme](../concepts/style_theme.md)。
