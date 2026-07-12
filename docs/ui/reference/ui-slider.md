# UiSlider

头文件：`engine/ui/widgets/ui_slider.h`。带内部 Bar、DragHandle、可选 Number 的数值控件。

调用：`set_slider_config` 原子配置；`set_range`、`set_value`、`set_step` 管理数值；`set_orientation` 选择横/竖；`set_value_display` 选择 None/Value/Percent；`set_on_value_changed` 接收 `float`。数值格式 API 包括 decimal places、trim zeros、target height 与 suffix。
