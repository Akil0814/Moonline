# UiButton

头文件：`engine/ui/widgets/ui_button.h`。可点击的文本、图标或状态纹理控件。

调用：使用 `UiButtonConfig` 构造或 `set_button_config` 原子更新；`set_text_content`、`set_icon_content`、`set_texture_set_content` 选择内容；`set_on_click` 注册点击回调。`set_state_textures`、`set_sounds`、`set_padding` 配置表现。样式/role 遵循[样式专题](../concepts/style-theme.md)。

`UiButtonContent` 与纹理指针均不转移外部纹理所有权。
