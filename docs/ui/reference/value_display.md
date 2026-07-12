# 数值展示控件：UiBar 与 UiNumber

覆盖头文件：`widgets/ui_bar.h`、`widgets/number/ui_number.h`、`number/ui_digit_renderer.h`。

## UiBar

`set_range(min, max)` 加 `set_value(value)` 将数值映射为 fill ratio；`set_ratio(ratio)` 直接设定 `[0, 1]` 范围的填充比例。`min_value`、`max_value`、`value`、`ratio` 查询最终状态。`set_fill_direction` 选择四个填充方向，`set_padding` 留出可绘制内边距。

## UiNumber

`set_value(double)` 更新数字；`set_decimal_places`、`set_trim_trailing_zeros`、`set_keep_decimal_point`、`set_suffix` 管理文本格式。`set_digit_spacing`、`set_fixed_glyph_advance`、`set_target_height` 管理字形几何；对应 `clear_*` 移除可选约束。

`UiDigitRenderer` 是底层 render-command 帮助类：向其提供 `UiDigitRenderRequest` 与 DigitCache，调用 `render` 追加命令。常规场景使用 UiNumber，不直接构造 renderer。
