# UiStyleState

头文件：`engine/ui/style/ui_style.h`。组件内部使用的泛型 base-style/override 合成器。调用 `reset`、`set_base_style`、`set_style_overrides`、`effective_style`、`has_style_overrides`、`clear_style_overrides`；应用代码优先使用具体组件的同名转发 API。
