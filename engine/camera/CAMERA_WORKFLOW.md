# 摄像机工作流程

本目录实现的是 Moonline 的 **2D 世界坐标摄像机**。它把游戏物体提交的世界坐标渲染命令投影到屏幕坐标；UI 使用独立的屏幕坐标渲染路径，因此不随摄像机移动。

当前模块分为三层：

```text
Scene
  ├─ Camera                 保存最终用于渲染的中心点和视口大小
  └─ CameraController       可选；计算跟随、边界限制和震屏
       ├─ IFollowStrategy   决定逻辑中心如何追随焦点
       └─ CameraEffect      为最终渲染中心叠加临时效果
```

## 文件职责

| 文件 | 职责 |
| --- | --- |
| `camera.h/.cpp` | 摄像机状态、可见世界区域，以及世界到屏幕的坐标投影。 |
| `camera_controller.h/.cpp` | 管理焦点、跟随策略、世界边界和单个活动特效；将最终结果写回 `Camera`。 |
| `follow_strategy.h/.cpp` | 三种无状态/轻状态的跟随算法：硬跟随、死区、匀速平滑。 |
| `camera_effect.h/.cpp` | 可扩展的摄像机效果接口；当前实现为衰减正弦震屏。 |

## 坐标与投影

`Camera` 只保存两个值：

- `center`：当前视口在世界坐标中的中心。
- `viewport_size`：视口宽高；设置时负值会被钳制为 `0`。

由此得到：

```text
view_rect = Rect::from_center(center, viewport_size)
screen_position = world_position - view_rect.top_left()
```

矩形投影只平移左上角，保持原始尺寸。因此当前实现：

- 支持平移；
- 不支持缩放、旋转或透视；
- 不在摄像机层做可见性裁剪；提交的世界渲染命令都会被投影并交给 SDL 执行。

在 `Scene::on_render` 中，每一个 `GameObject` 深度层的 `RenderCommand` 都会先经过上述投影，再执行渲染。之后 UI 命令直接执行，不经过 `Camera`，所以 UI/HUD 不会随世界滚动或震屏。

## 每帧更新顺序

`Scene::on_update(delta)` 在普通对象更新、UI 展示动画、物理和碰撞之后，按以下顺序更新摄像机：

```text
1. Scene::resolve_camera_focus_rect()
2. CameraController::set_focus_rect(...)
3. CameraController::update(delta)
   3.1 首次获得焦点时，立即对齐焦点中心
   3.2 后续帧按跟随策略计算 logical_center
   3.3 将 logical_center 限制在 world_bounds 内
   3.4 叠加活动 CameraEffect 的偏移
   3.5 将 final_render_center 写入 Camera::center
4. 下一次 on_render 使用 Camera::center 投影世界命令
```

控制器不存在时，`Scene` 仍会使用其持有的 `Camera` 渲染；此时中心完全由调用方直接设置。

## 焦点与跟随策略

焦点是一个可选的世界矩形，而不是对象指针。场景应覆写 `resolve_camera_focus_rect()`，在每帧返回要关注对象当前的世界矩形；没有焦点时控制器保留现有逻辑中心。

### 首次对焦

控制器第一次收到焦点时，不经过跟随策略，直接把逻辑中心设为焦点矩形中心。这避免了场景刚进入时摄像机从默认位置缓慢追赶目标。

### `HardFollowStrategy`

每帧直接返回 `focus_rect.center()`。适用于始终把目标固定在屏幕中央的场景。

### `DeadZoneFollowStrategy`

死区矩形以屏幕/视口局部坐标表示。只要焦点矩形仍完全处于死区内，摄像机保持不动；焦点越过任一边时，摄像机仅移动足以使该边重新贴合死区边缘的距离。

若死区为空，或焦点本身大于死区，策略退化为硬跟随，避免无法将焦点完整放入死区的情况。

### `SmoothFollowStrategy`

以 `follow_speed_units_per_second * delta_seconds` 为本帧最大移动距离，向焦点中心做匀速直线移动；距离不足一个步长时直接到达。速度或 `delta_seconds` 非正时不移动。

## 世界边界

`CameraController::set_world_bounds()` 接收一个可选世界矩形。存在边界时，控制器会对逻辑中心做轴向限制：

- 世界宽/高大于视口：中心限定在使视口不越过世界边界的范围内。
- 世界宽/高小于等于视口：对应轴始终使用世界中心，使小地图位于视口中央。

边界约束发生在跟随之后、震屏之前。因此震屏允许最终渲染中心暂时越过逻辑世界边界；这保留了震动的完整视觉效果，而不会改变跟随的逻辑位置。

## 震屏效果

`start_shake(params)` 替换当前活动效果。当前 `CameraShakeEffect`：

- 对振幅、持续时间和频率做非负钳制；
- 以不同频率的正弦/余弦生成 X/Y 偏移；
- 用线性包络让振幅从初始值衰减到 `0`；
- 持续时间结束后控制器自动清除效果。

`clear_shake()` 会立即移除效果并把渲染中心恢复到逻辑中心。控制器当前只持有一个 `CameraEffect`，后一次 `start_shake()` 会取代前一次，而不是叠加多个效果。

## 场景接入要求

本仓库当前已经把投影路径接入 `Scene`，但尚未发现具体场景创建 `CameraController`、设置视口大小，或提供焦点矩形。因此默认 `viewport_size` 为 `(0, 0)`，投影结果等同于不做平移。

一个需要跟随的场景应在初始化阶段完成：

```cpp
set_camera_viewport_size({ logical_width, logical_height });

auto* controller = emplace_camera_controller();
controller->set_follow_strategy(
    std::make_unique<elysia::camera::SmoothFollowStrategy>(300.0)
);
controller->set_world_bounds(world_rect); // 若地图边界已知
```

并覆写：

```cpp
std::optional<elysia::core::Rect> MyScene::resolve_camera_focus_rect() const
{
    return _player ? std::optional(_player->world_rect()) : std::nullopt;
}
```

若窗口或逻辑分辨率可在运行时变化，应再次调用 `set_camera_viewport_size()`。该函数会同步更新摄像机，并在有控制器时重新应用世界边界约束。

## 扩展边界

- 新的追随行为：实现 `IFollowStrategy`，不需要修改 `CameraController`。
- 新的视觉效果：实现 `CameraEffect`；若需要多效果叠加，需要把控制器当前的单一 `_active_effect` 改为效果集合。
- 缩放、旋转或裁剪：属于 `Camera` 与渲染投影层的扩展，不应放入跟随策略。
- 将鼠标屏幕坐标转换回世界坐标：当前未提供 `screen_to_world`，需要时应作为 `Camera` 的对称转换接口加入。
