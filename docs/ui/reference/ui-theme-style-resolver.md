# UiThemeStyleResolver

头文件：`engine/ui/style/ui_theme_style_resolver.h`。将 `UiTheme` 映射到 element base styles。调用 `register_adapter` 添加类型适配器，`apply` 应用单个 element，`apply_subtree` 应用树；通常由 `UiThemeManager` 管理。
