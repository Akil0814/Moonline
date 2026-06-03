# engine/input 输入系统架构与工作流

本文基于当前 `engine/input`、`application`、`engine/core/scene`、以及部分 UI/Scene 调用点的真实实现整理，目标是说明这个输入系统现在是怎么分层的、每帧如何流转、以及状态输入和事件输入分别适合做什么。

## 1. 这个文件夹里各文件的职责

### `input_types.h`

定义输入系统的基础数据类型：

- `InputAction`
  - 统一的游戏语义动作，比如 `Left`、`Right`、`Confirm`、`Cancel`、`Attack`、`Dash`、`Backspace`、`DeleteKey`、`Exit`。
- `InputContext`
  - 输入上下文，当前有 `Gameplay`、`UI`、`Dialogue`、`Debug`。
- `InputDevice`
  - 当前输入设备类型：`Keyboard`、`Mouse`、`Gamepad`、`Unknown`。
- `InputEventType`
  - 离散事件类型：`Pressed`、`Released`、`MouseWheel`、`TextInput`、`TextEditing`。
- `InputEvent`
  - 系统内部统一事件格式。除了 `action/type/device`，还可以携带滚轮值 `wheel_x/wheel_y` 和文本输入 `text`。

这层的意义是先把 SDL 的原始输入，抬高成项目自己的“语义动作 + 离散事件”。

## 2. `InputState`：保存“当前帧持续状态”

`input_state.h` 是一个非常轻量的状态容器，内部维护两份布尔数组：

- `_current`
- `_previous`

它负责解决“这个动作现在是不是按着”“是不是刚按下”“是不是刚松开”这类连续状态判断。

它提供的核心能力：

- `begin_frame()`
  - 把 `_current` 复制给 `_previous`，作为新一帧的状态基线。
- `set_pressed(action, pressed)`
  - 更新某个动作当前是否按下。
- `is_pressed(action)`
  - 当前是否按下。
- `is_just_pressed(action)`
  - 当前帧按下且上一帧没按下。
- `is_just_released(action)`
  - 当前帧松开且上一帧是按下。

所以 `InputState` 本质上是“面向轮询查询”的输入快照核心。

## 3. `InputTranslator`：把 SDL 事件翻译成项目事件

`InputTranslator` 是一个 `Singleton`，只做一件事：

- `translate_event(const SDL_Event&) -> std::vector<InputEvent>`

也就是说，它不保存帧状态，不负责分发，只负责“翻译”。

### 当前映射规则

#### 键盘

- `A / Left` -> `InputAction::Left`
- `D / Right` -> `InputAction::Right`
- `W / Up` -> `InputAction::Up`
- `S / Down` -> `InputAction::Down`
- `Enter / Keypad Enter` -> `Confirm`
- `Esc` -> `Cancel`
- `P` -> `Pause`
- `Space` -> `Jump`
- `J` -> `Attack`
- `K` -> `Special`
- `L` -> `Guard`
- `LeftShift / RightShift` -> `Dash`
- `Backspace` -> `Backspace`
- `Delete` -> `DeleteKey`
- `Home` -> `Home`
- `End` -> `End`
- `Tab` -> `Tab`

并且：

- `SDL_KEYDOWN` 的重复输入会被忽略。
- 也就是长按键盘时，不会因为 SDL 的 repeat 机制重复生成 `Pressed` 事件。

#### 鼠标

- 左键 -> `Attack`
- 右键 -> `Guard`
- 滚轮 -> `MouseWheel`

这里鼠标按键被映射成了游戏动作，但鼠标位置本身没有进入 `InputEvent`，UI 鼠标位置还是直接从 SDL/鼠标工具层取。

#### 手柄按钮

- DPad 四方向 -> `Left/Right/Up/Down`
- `A` -> 同时生成 `Confirm` 和 `Jump`
- `B` -> `Cancel`
- `X` -> `Attack`
- `Y` -> `Special`
- `LB` -> `Guard`
- `RB` -> `Dash`
- `Start` -> `Pause`

这里有个很重要的特点：

- 一个 SDL 事件可以翻译成多个 `InputEvent`。
- 例如手柄 `A` 会同时发出 `Confirm` 和 `Jump`。

#### 文本与退出

- `SDL_TEXTINPUT` -> `TextInput`
- `SDL_TEXTEDITING` -> `TextEditing`
- `SDL_QUIT` -> `InputAction::Exit` + `Pressed`

## 4. `InputSystem`：真正的帧级输入管理器

`InputSystem` 是 `engine/input` 的核心协调层。它把三个事情串起来：

1. 帧生命周期管理
2. SDL 事件处理
3. 当前帧快照与事件列表输出

它内部主要维护这些成员：

- `_state`
  - `InputState`，存连续按键状态。
- `_events`
  - 当前帧收集到的 `InputEvent` 列表。
- `_context`
  - 当前输入上下文。
- `_current_device`
  - 当前活跃设备。
- `_device_switched_this_frame`
  - 本帧是否刚切换设备。
- `_left_stick_x / _left_stick_y`
  - 手柄左摇杆轴值缓存。
- `_gamepad_scroll_accumulator`
  - 把摇杆连续输入累积成离散滚轮步进时使用。

### 4.1 帧开始：`begin_frame()`

每帧一开始：

- `_state.begin_frame()`
- `_events.clear()`
- `_device_switched_this_frame = false`

这一步的意义是：

- 先固定上一帧状态
- 清空本帧离散事件
- 开始接收新一帧输入

### 4.2 逐个 SDL 事件处理：`process_event()`

每拿到一个 `SDL_Event`，`InputSystem` 会做这些事：

1. `update_controller_axis_state(event)`
   - 如果是 `SDL_CONTROLLERAXISMOTION`，先缓存左摇杆的 X/Y。
2. `detect_event_device(event)`
   - 判断这次事件来自键盘、鼠标还是手柄。
3. 维护 `_current_device`
   - 第一次输入会直接确定当前设备。
   - 键鼠和手柄切换时会清理部分状态，避免旧设备残留状态污染新设备。
4. `translate_event(event)`
   - 交给 `InputTranslator` 做 SDL -> `InputEvent` 翻译。
5. `append_event(event)`
   - 逐个写入 `_events`，同时把 `Pressed/Released` 类型同步到 `_state`。

所以它不是“只有状态”也不是“只有事件”，而是同时维护：

- 一份适合轮询的 `InputState`
- 一份适合逐次处理的 `std::vector<InputEvent>`

### 4.3 设备切换策略

当前设备切换逻辑比较明确：

- 从未知设备收到第一条有效输入时，直接设置 `_current_device`
- 从键盘/鼠标切到手柄时：
  - 设置 `_device_switched_this_frame = true`
  - 清空 `_state`
  - 清空手柄滚动累积
  - 并且这次事件会直接 `return`
- 从手柄切回键盘或鼠标时：
  - 清空 `_state`
  - 重置手柄滚动状态
  - 再继续处理当前事件

这个策略的目标是避免：

- 比如上一设备的“按住状态”残留到下一设备
- 或者刚切手柄那一瞬间 UI 立即吃到一次多余输入

## 5. `end_frame()`：把左摇杆补成 UI 滚轮事件

`end_frame()` 目前只做一件事：

- `emit_ui_gamepad_scroll()`

这部分是当前实现里最“带业务语义”的地方之一。

### 5.1 它什么时候生效

只有同时满足下面条件才会产出滚轮事件：

- `InputContext` 是 `UI`
- 当前设备是 `Gamepad`
- 本帧不是刚发生设备切换
- 左摇杆 Y 轴越过死区

### 5.2 它怎么工作

逻辑大致是：

1. 对左摇杆 X/Y 做死区和归一化处理。
2. 用 Y 作为主滚动方向。
3. 用 X 的绝对值做一个“垂直权重”削减，避免斜向输入过强。
4. 把连续轴值累加进 `_gamepad_scroll_accumulator`。
5. 当累积值达到整数步进时，生成 `MouseWheel` 事件。
6. 每帧最多夹到 `[-3, 3]` 步。

最终生成的是：

- `InputEventType::MouseWheel`
- `device = InputDevice::Gamepad`
- `wheel_y = wheel_steps`

### 5.3 一个很关键的现状

当前左摇杆并不会直接翻译成 `Left/Right/Up/Down` 动作。

现在它只做两件事：

- 在 `process_event()` 里缓存轴值
- 在 `end_frame()` 且 `UI` 上下文下，把它转成滚轮事件

也就是说：

- UI 列表/滚动容器可以借这个机制吃到手柄滚动
- 但“摇杆推一下就变成方向动作”这条链路目前并不存在

## 6. `InputSnapshot` 和 `events()`：为什么系统同时输出两套输入数据

`InputSystem` 对外暴露两套输入结果：

### `snapshot()`

返回 `InputSnapshot`，里面包含：

- `const InputState& state`
- `InputContext context`
- `InputDevice device`
- `bool device_switched_this_frame`

这一套适合做“持续状态查询”，例如：

- 角色是否持续按住左键移动
- 当前焦点控件是否收到方向键按下
- 当前输入设备是不是手柄

### `events()`

返回当前帧的 `std::vector<InputEvent>`

这一套适合做“离散事件消费”，例如：

- 文本输入字符
- 鼠标滚轮
- 某个动作的单次触发
- 退出事件

这是现在架构里很重要的一个分层：

- `InputSnapshot` 负责“状态”
- `InputEvent` 列表负责“事件”

## 7. 整个输入系统的真实工作流

当前真实工作流不是只发生在 `engine/input`，而是这样贯穿整个程序：

### 第 1 步：`Application` 持有 `InputSystem`

`application/application.h` 中有：

- `InputSystem _input_system;`

并且在 `Application::init()` 里，当前会先执行：

- `_input_system.set_context(InputContext::UI);`

注意这里的现状：

- 当前代码里只有这一次显式设置上下文
- 也就是现在运行时默认一直处于 `UI` 上下文，除非后续代码新增切换

### 第 2 步：主循环开始一帧输入采集

`Application::run()` 中每帧先：

1. `_input_system.begin_frame()`
2. `while (SDL_PollEvent(&_event))`

循环里会做三件事：

1. `SDL_QUIT` 时把 `_active` 设为 `false`
2. `handle_controller_device_event(_event)`
   - 管手柄插拔
3. `_input_system.process_event(_event)`
   - 把 SDL 事件送入输入系统

### 第 3 步：事件轮询结束后做帧末补充

轮询结束后：

- `_input_system.end_frame()`

这里主要是让左摇杆在 UI 上下文下补发滚轮事件。

### 第 4 步：把输入交给场景系统

随后 `Application` 调用：

- `SceneManager::instance()->on_input(_input_system.snapshot(), _input_system.events());`

所以场景层拿到的是：

- 一份状态快照
- 一份离散事件列表

### 第 5 步：`Scene` 双通道分发给每个对象

`engine/core/scene/scene.h` 的默认实现会对每个活动对象做两次分发：

1. `obj->on_input(input);`
2. 对 `events` 中每个事件执行 `obj->on_input_event(input_event);`

这说明当前对象层输入接口天然分成两条：

- `on_input(const InputSnapshot&)`
  - 面向状态查询
- `on_input_event(const InputEvent&)`
  - 面向离散事件处理

## 8. 现有调用方式说明：哪些东西走状态，哪些东西走事件

看当前代码，已经形成了比较清晰的使用习惯。

### 8.1 走 `on_input` / `InputSnapshot` 的场景

更适合处理“持续态”或“每帧态”的逻辑。

例子：

- UI 控件根据当前输入设备决定是否允许鼠标输入
- 焦点控件根据 `is_pressed / is_just_pressed` 做导航
- 对象基于当前设备状态决定表现

`UiTextInput::on_input()` 就是一个例子：

- 它不从 `InputEvent` 里拿鼠标点击
- 而是每帧结合 `InputSnapshot` 和 UI 鼠标工具函数判断鼠标释放是否发生在自身区域内

这说明鼠标位置/命中检测，目前仍然主要依赖 UI 鼠标工具层，而不是统一塞进 `InputEvent`。

### 8.2 走 `on_input_event` / `InputEvent` 的场景

更适合处理“离散事件”。

典型例子：

- `UiTextInput`
  - `TextInput` 事件插入文字
  - `Backspace/DeleteKey/Home/End/Left/Right` 处理光标和编辑
- `UiScrollPanel`
  - 消费 `MouseWheel` 事件进行滚动
- `OptionsScene`
  - 遍历本帧 `events`，收到 `Cancel` 就返回主菜单

所以：

- 需要“字符输入”“滚轮”“单次确认/取消”的地方，基本应该优先看事件流
- 需要“持续按住”“刚按下/刚松开”的地方，优先看状态流

## 9. 这个架构的优点

### 优点 1：SDL 被隔离在翻译层和系统层

业务对象不必直接理解复杂 SDL 事件类型，基本只需要处理：

- `InputSnapshot`
- `InputEvent`

### 优点 2：同时支持状态型输入和事件型输入

这是当前设计最合理的地方之一，因为很多输入场景天然不是一类：

- 角色移动、焦点导航更像状态
- 文本输入、滚轮、一次性触发更像事件

### 优点 3：设备切换有显式治理

它不仅知道“收到什么输入”，还知道“当前是谁在输入”，并处理了切换时的状态清理。

### 优点 4：UI 对手柄滚动有专门兼容

左摇杆被转成滚轮事件后，现有很多基于 `MouseWheel` 的 UI 滚动逻辑可以直接复用，不需要为手柄再写一套滚动协议。

## 10. 当前实现里值得注意的边界与限制

### 1. `InputContext` 现在基本是静态的

虽然设计上支持 `Gameplay/UI/Dialogue/Debug`，但当前实际只在 `Application::init()` 里设过一次 `UI`。

这意味着：

- 上下文系统已经预留好了
- 但“按场景或界面动态切换上下文”的机制现在还没有真正铺开

### 2. 左摇杆目前不是通用方向输入

左摇杆现在不会直接变成：

- `Left`
- `Right`
- `Up`
- `Down`

它当前只参与 UI 滚轮事件的生成。

### 3. 鼠标位置不在统一事件里

`InputEvent` 里有动作、滚轮、文本，但没有鼠标坐标。

所以 UI 点击、悬停、区域命中，依然依赖额外的鼠标工具函数，而不是完全由输入系统统一封装。

### 4. 手柄 `A` 同时映射为 `Confirm` 和 `Jump`

这对 UI 和 Gameplay 复用输入很方便，但也意味着：

- 如果未来某些场景不希望 `A` 同时代表两种语义
- 就需要依赖 `InputContext` 或更细的映射策略继续拆分

## 11. 一句话总结

当前 `engine/input` 的架构可以概括成：

- `input_types.h` 定义统一输入语义
- `InputTranslator` 负责 SDL 事件到项目事件的翻译
- `InputSystem` 负责帧生命周期、设备切换、状态缓存、事件收集
- `Application` 在主循环里驱动输入采集
- `Scene` 再把“状态快照 + 离散事件”双通道分发给对象和 UI

所以它本质上是一个：

- “SDL 原始事件 -> 项目语义事件 -> 帧级状态/事件输出 -> Scene/Object 分发”

的分层输入系统。

## 12. 如果后面要继续扩展，最自然的演进点

如果以后继续演进，这个架构最自然的扩展点大概有这些：

1. 让 `InputContext` 跟随场景/UI 状态动态切换，而不是初始化后固定。
2. 给手柄左摇杆增加方向动作或导航动作翻译，而不只生成滚轮。
3. 把按键映射从硬编码 `switch` 提取成可配置绑定。
4. 如果 UI 统一化程度还要提高，可以考虑把鼠标位置/点击语义进一步纳入输入系统，而不是分散在 UI 鼠标工具中。

