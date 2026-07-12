# UiDigitRenderer

头文件：`engine/ui/number/ui_digit_renderer.h`。把 `UiDigitRenderRequest` 转为数字 render commands。构造时可传 DigitCache；`set_digit_cache` 更换缓存；`render(request, out_commands)` 追加命令。通常由 `UiNumber` 内部使用。
