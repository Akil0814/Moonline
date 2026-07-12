# UiImage

头文件：`engine/ui/widgets/image/ui_image.h`。借用 `SDL_Texture*` 的图像元素。

调用：构造器接受 texture 与几何；`set_texture`/`texture` 替换或读取借用纹理；`set_source_rect` 裁剪源区域，`clear_source_rect` 恢复整图。Fade/Blink/Pulse image 均继承这些 API，并增加 opacity playback API，见[效果专题](../concepts/effects.md)。
