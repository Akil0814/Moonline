# UiAnimation

头文件：`engine/ui/widgets/image/ui_animation.h`。展示已注册帧动画的元素。

调用：构造或 `set_animation_key` 绑定 key；后者失败返回 `false`。`set_loop` 覆盖循环；`play`、`pause`、`resume`、`reset` 控制播放；`is_finished`、`is_paused` 查询状态。调用前确保 AnimationManager 已注册该 key。
