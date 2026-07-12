# UiSlider

头文件：`engine/ui/widgets/ui_slider.h`。带内部 Bar、DragHandle、可选 Number 的数值控件。

调用：`set_slider_config` 原子配置；`set_range`、`set_value`、`set_step` 管理数值；`set_orientation` 选择横/竖；`set_value_display` 选择 None/Value/Percent；`set_on_value_changed` 接收 `float`。数值格式 API 包括 decimal places、trim zeros、target height 与 suffix。

`set_range(min, max)` 要在设置 value 前调用；实现会处理反向/退化范围并 clamp value。`set_step(std::nullopt)` 允许连续值，非空正值会让键盘/手柄步进与赋值结果对齐到 step。Value callback 仅在实际值变化时触发。
