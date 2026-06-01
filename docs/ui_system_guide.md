# UI 系统说明与工作流

## 1. 当前 UI 系统的定位

当前项目的 UI 系统已经不是“零散控件集合”，而是一个可复用的 2D UI 框架雏形。

它现在已经覆盖了以下能力：

- 页面级容器和过渡动画
- 线性布局、滚动容器、基础二维网格布局
- 通用可聚焦控件体系
- 鼠标、键盘、手柄共同走一套 UI 输入动作
- 基础控件、复合控件、表单控件
- 主菜单、选项页、表单演示页这类真实页面落地

但它还不是一个“完全封箱”的成熟 UI 框架。

当前最准确的定位是：

- 适合继续扩展菜单、设置页、对话框、简单 HUD、表单页
- 架构已经足够承载后续 UI 开发
- 主题系统、复杂焦点图、更多高级控件仍有继续增强空间

## 2. 当前 UI 架构分层

### 2.1 最底层：`GameObject`

UI 系统最底层仍然建立在 `GameObject` 之上。

关键点：

- 所有 UI 组件本质上都是 `GameObject` 或间接建立在 `GameObject` 上
- `GameObject` 提供：
  - 世界坐标
  - 尺寸
  - `SDL_Rect`
  - `on_update`
  - `on_render`
  - `on_input`
  - `on_input_event`

这意味着 UI 系统没有单独发明一套完全独立的对象生命周期，而是直接复用了游戏对象体系。

### 2.2 交互层：`UiFocusable`

当前交互式 UI 的统一接口是 `engine/ui/ui_focusable.h` 里的 `UiFocusable`。

它定义了交互控件需要实现的最小契约：

- `set_focused(bool focused)`
- `is_focused() const`
- `set_enabled(bool enabled)`
- `is_enabled() const`
- `handle_focused_input_event(const InputEvent& event)`
- `game_object()`

它的意义很重要：

- `UiScreen` 不再只认识 `Button`
- 任何可以被焦点系统选中的 UI 控件，都可以挂进同一套导航体系
- `Button`、`UiMenuList`、`UiOptionList`、`UiSlider`、`UiToggle`、`TextInput` 都已经走这套模型

### 2.3 布局与容器层

#### `UILayout`

`engine/ui/ui_layout.h`

这是当前最核心的 UI 容器基类。

它负责：

- 管理 child 列表
- 排列方向：`Horizontal / Vertical`
- 锚点：`TopLeft / Center / BottomRight` 等
- 副轴对齐：`Start / Center / End`
- 间距、padding、child margin
- child 尺寸覆盖
- `fill_cross_axis`
- 容器 transform
- 内容偏移 `content_offset`
- auto size

当前 `UILayout` 的特点是：

- 它不是通用场景树
- 它只负责自己内部 child 的更新、渲染、输入转发
- child 使用相对容器计算出的世界坐标

换句话说，它是“轻量 UI 容器层”，不是整个引擎的父子节点系统。

#### `UiPanel`

`engine/ui/containers/ui_panel.h`

`UiPanel` 继承 `UILayout`，在布局能力上再加：

- 背景颜色
- 边框
- 背景贴图
- 背景 alpha
- child 裁剪

它适合作为：

- 面板
- 卡片
- 列表背景
- 设置行容器
- 弹窗主体

#### `UiScrollPanel`

`engine/ui/containers/ui_scroll_panel.h`

它在 `UiPanel` 基础上增加：

- `scroll_offset`
- `scroll_step`
- 横向/纵向滚动开关
- 滚动范围夹紧
- `ensure_child_visible`
- 鼠标滚轮滚动

它的本质是：

- 仍然使用 `UILayout` 做排版
- 通过 `content_offset` 实现视口内容偏移

#### `UiGridLayout`

`engine/ui/layout/ui_grid_layout.h`

这是补进来的基础二维布局。

当前能力：

- 指定列数
- 自动换行
- 行列间距
- 容器 padding
- 单元格内水平/垂直对齐

这适合做：

- 表单
- 多列按钮组
- 简单图标网格
- 选项面板的 label/control 二列排版

### 2.4 页面层：`UiScreen`

`engine/ui/containers/ui_screen.h`

`UiScreen` 是当前页面级 UI 的核心。

它在 `UiPanel` 基础上增加：

- 打开/关闭
- 过渡动画
- 输入开关
- 通用可聚焦控件注册
- 默认线性焦点导航
- 鼠标点选控件后聚焦

当前 `UiScreen` 的角色是：

- 一个 scene 中的 UI 页面根对象
- 负责管理页面内的焦点目标
- 负责页面级开合和过渡

它不是 scene manager，也不是全局 UI manager，而是“单页容器”。

## 3. 控件分层

### 3.1 基础显示控件

当前已有：

- `Label`
- `ImageView`
- `Bar`
- `ProgressBar`

它们主要负责显示，不负责复杂交互。

其中：

- `Label` 负责文本、字体、颜色、padding、对齐、换行
- `ImageView` 负责图片显示
- `Bar` 是通用数值条绘制核心
- `ProgressBar` 是 `Bar` 的 UI 外壳

### 3.2 交互控件

当前已有：

- `Button`
- `TextButton`
- `UiSlider`
- `UiToggle`
- `TextInput`

职责分别是：

- `Button`：最基础按钮，支持鼠标点击和焦点确认
- `TextButton`：带文字渲染的按钮
- `UiSlider`：连续值输入，支持拖拽、点击轨道、方向键/手柄调整
- `UiToggle`：布尔开关
- `TextInput`：单行文本输入，支持 placeholder、密码模式、最大长度、光标移动、删除

### 3.3 滚动和辅助控件

当前已有：

- `UiScrollBar`

它现在支持：

- 跟随 `UiScrollPanel`
- 显示 thumb 和 track
- 鼠标拖拽 thumb
- 点击轨道跳转
- 自动隐藏

注意：

- 当前 `UiScrollBar` 更像附属控件
- 它通常作为单独对象添加到 scene，而不是总是作为 panel 内 child 参与布局

### 3.4 复合控件

当前已有：

- `UiMenuList`
- `UiOptionList`
- `UiDialog`

#### `UiMenuList`

基于 `UiScrollPanel + TextButton`。

它负责：

- 菜单项生成
- 选中项管理
- 上下切换
- 焦点高亮
- 确认回调
- 选中项自动滚到可见区域

#### `UiOptionList`

基于 `UiScrollPanel + UiPanel + Label + (UiSlider / UiToggle)`。

它负责：

- 设置项行列表
- 行选中
- 行内控制件焦点同步
- `Toggle` 和 `Slider` 混合承载
- 值变化回调

它已经不是旧版那种“纯文本左右切值”列表，而是复合控件容器。

#### `UiDialog`

基于 `UiScreen + Label + UiMenuList`。

它负责：

- 标题
- 正文
- 操作项列表
- `Cancel` 关闭/取消逻辑

当前对话框本质上是一个模态风格页面控件。

## 4. 输入流与焦点流

### 4.1 输入来源

当前 UI 输入来自 SDL 事件，核心转换在：

- `engine/input/input_translator.cpp`
- `engine/input/input_system.cpp`

当前已经支持：

- 键盘方向键
- 回车、Esc、Tab、Backspace、Delete、Home、End
- 鼠标按键和滚轮
- `SDL_TEXTINPUT`
- `SDL_TEXTEDITING`
- 手柄 `DPad`
- 手柄 `A/B/Start`

当前输入动作是统一抽象的，比如：

- `Left`
- `Right`
- `Up`
- `Down`
- `Confirm`
- `Cancel`

所以大部分 UI 控件都不需要自己区分“这是键盘还是手柄”。

### 4.2 运行时输入传播

运行时路径大致是：

1. `Application::run()` 轮询 SDL 事件
2. `InputSystem` 接收事件
3. `InputTranslator` 把 SDL 事件转换成 `InputEvent`
4. `SceneManager` 把 `InputSnapshot + events` 传给当前 scene
5. `Scene::on_input()` 继续把输入转发给 scene 内对象
6. `UiScreen / UILayout / 各种控件` 消费输入

### 4.3 焦点处理规则

当前焦点规则是：

- `UiScreen` 维护一组已注册的 `UiFocusable`
- 页面打开后可设置默认焦点
- 鼠标点击控件 rect 时，控件会被切成当前焦点
- 当前焦点控件优先处理输入
- 如果控件没有消费某个按键，`UiScreen` 才可能把它当成“切换焦点”指令

当前导航策略偏简单：

- 线性导航为主
- 竖向页面通常用 `Up/Down`
- 横向页面通常用 `Left/Right`
- `Confirm` 触发当前控件主行为

还没有完整的“按空间位置找最近邻”的二维焦点图。

## 5. 样式层和主题现状

当前已有统一样式层：

- `ButtonStyle`
- `LabelStyle`
- `PanelStyle`
- `BarStyle`
- `ScrollBarStyle`
- `SliderStyle`
- `ToggleStyle`
- `TextInputStyle`

并通过 `UiStyle::apply_*()` 应用到控件。

这意味着：

- 样式结构已经统一
- 控件外观不再完全靠散乱 setter 临时拼装

但要注意：

- 当前没有真正的 `UiTheme` / `UiThemeManager`
- 页面里仍然有很多颜色直接写在 scene 的 `reset()` 里
- 所以现在是“统一样式格式”，不是“统一主题来源”

如果后面要做全局换肤，建议再加一层：

- `UiTheme`
- `UiThemeManager`
- `MainMenuTheme`
- `HudTheme`

## 6. 当前已有的真实页面

### `MainMenuScene`

主菜单页，主要演示：

- 页面根 `UiScreen`
- `Label`
- `UiMenuList`
- `UiScrollBar`
- `UiDialog`

### `OptionsScene`

设置页，主要演示：

- 页面根 `UiScreen`
- `UiOptionList`
- `UiScrollBar`
- `Slider + Toggle` 混合设置项

### `UiFormsDemoScene`

表单演示页，主要演示：

- `UiGridLayout`
- `TextInput`
- `UiToggle`
- `UiSlider`
- `TextButton`

它很适合作为今后新增 UI 时的参考模板。

## 7. 如果要创建一个新的 UI，推荐工作流程

下面是当前项目最自然、最稳定的 UI 创建流程。

### 7.1 明确 UI 属于哪一层

先判断你要做的东西属于哪一类：

- 单个控件：直接写 `widget`
- 复合列表/弹窗：写 `composite`
- 面板/容器：写 `containers` 或 `layout`
- 完整页面：写 scene

推荐目录习惯：

- 基础控件：`engine/ui/widgets`
- 复合控件：`engine/ui/composite`
- 容器：`engine/ui/containers`
- 布局：`engine/ui/layout`
- 页面入口：`gameplay/scene`

### 7.2 如果只是加一个新基础控件

比如你要做 `UiDropdown`。

推荐步骤：

1. 让它继承 `GameObject`
2. 如果它可以被焦点选中，再实现 `UiFocusable`
3. 补齐：
   - `on_update`
   - `on_render`
   - `on_input`
   - `reset`
   - `handle_focused_input_event`
4. 如果它有统一外观需求，再加一个 style 结构体并接入 `UiStyle`

如果它只是一个显示件，不需要实现 `UiFocusable`。

### 7.3 如果要做一个新复合控件

比如你要做：

- 角色状态列表
- 武器选择列表
- 任务列表

推荐步骤：

1. 先选一个基础容器：
   - 线性内容：`UiScrollPanel`
   - 一般面板：`UiPanel`
   - 页面级：`UiScreen`
2. 内部 child 用已有控件拼装
3. 把复合控件自己的状态抽象出来
4. 如果整个复合控件需要参与页面焦点导航，就让复合控件自己实现 `UiFocusable`
5. 不要把 scene 逻辑直接写进控件内部

现在的 `UiMenuList` 和 `UiOptionList` 都是这条路线。

### 7.4 如果要做一个新页面

比如你要做：

- `PauseMenuScene`
- `CharacterStatusScene`
- `InventoryScene`

推荐步骤：

1. 在 `gameplay/scene` 下新建 scene
2. scene 内保留 UI 成员指针
3. 使用 `ensure_ui()` 懒创建对象
4. 使用 `reset()` 统一重建页面状态
5. 以 `UiScreen` 作为页面根
6. 给 `UiScreen` 添加 child
7. 注册可聚焦控件
8. 最后把 `UiScreen` 和必要的附属对象 `add_object()` 到 scene

推荐结构：

```cpp
class MyScene : public Scene
{
private:
    std::shared_ptr<UiScreen> _screen;
    std::shared_ptr<Label> _title_label;
    std::shared_ptr<UiMenuList> _menu_list;
};
```

推荐页面搭建顺序：

1. `ensure_ui()`
2. `reset()`
3. 先配 `UiScreen`
4. 再配控件内容和样式
5. 再配 layout child options
6. 再注册焦点
7. 最后 `open()` 和 `add_object()`

### 7.5 如果要做表单类 UI

比如名字输入、设置页、登录框。

推荐组合：

- label + input：`UiGridLayout`
- 开关：`UiToggle`
- 连续值：`UiSlider`
- 提交按钮：`TextButton`

最适合参考的现成实现就是：

- `UiFormsDemoScene`

### 7.6 如果要做设置页

当前推荐直接复用 `UiOptionList`。

一个设置项只要决定：

- `id`
- `label`
- `control_type`
- toggle 还是 slider 的初始值
- 是否 enabled

当前最自然的定义方式就是：

```cpp
UiOptionListItem item;
item._id = "bgm_volume";
item._label = "BGM Volume";
item._control_type = UiOptionControlType::Slider;
item._slider = { 0.0f, 100.0f, 75.0f, 5.0f, 0, "%" };
```

## 8. 创建新 UI 时的推荐原则

### 8.1 优先组合，不要乱堆继承

例如：

- 数值显示优先用 `Bar + 外壳`
- 复合设置项优先用 `Label + Slider/Toggle`

不要动不动做深继承树。

### 8.2 页面逻辑留在 scene，控件逻辑留在控件

控件应该负责：

- 渲染
- 交互
- 自己的小状态

scene 应该负责：

- 这个页面有什么控件
- 控件之间怎么联动
- 页面切换到哪里
- scene 级回调和数据流

### 8.3 先走现有输入动作，不要绕过 `InputSystem`

现在 UI 键盘、鼠标、手柄已经基本走到同一套输入模型了。

新增 UI 时，优先复用：

- `InputAction::Up`
- `InputAction::Down`
- `InputAction::Left`
- `InputAction::Right`
- `InputAction::Confirm`
- `InputAction::Cancel`

不要在控件内部直接写死大量 SDL 原始键值判断，除非是 `TextInput` 这种必须吃文本事件的特例。

### 8.4 样式尽量先集中成局部 style 变量

即使当前还没有全局主题系统，也建议在页面里先这样写：

- `PanelStyle screen_style`
- `ButtonStyle button_style`
- `LabelStyle title_style`

这样后面要抽主题时，迁移成本会低很多。

## 9. 当前系统的已知限制

当前仍有几个重要限制需要心里有数：

### 9.1 不是完整主题系统

当前只有 `UiStyle`，没有真正的全局主题管理器。

### 9.2 焦点导航仍然偏线性

复杂网格式页面如果要做“按空间方向找邻居”，还需要继续增强。

### 9.3 `Scene` 仍然是扁平对象列表

通用父子场景树没有建立起来。

所以很多 UI 容器都采用：

- 对外是一个顶层对象
- 对内自己维护 child

这条路线目前是符合项目现状的。

### 9.4 `LoadingScene` 还没有完全接到新 UI 体系

当前加载页仍然比较偏旧测试结构，后续如果要正式化，建议改造成：

- `UiScreen`
- `Label`
- `ProgressBar`
- 真正的加载状态绑定

## 10. 一句话总结

当前 UI 系统已经具备下面这条完整链路：

- `Application` 初始化 SDL 和输入
- `InputSystem / InputTranslator` 统一输入动作
- `Scene` 承载顶层 UI 页面对象
- `UiScreen` 负责页面生命周期和焦点
- `UILayout / UiPanel / UiScrollPanel / UiGridLayout` 负责布局和容器
- `Button / Slider / Toggle / TextInput` 负责基础交互
- `UiMenuList / UiOptionList / UiDialog` 负责高层复合行为
- `UiStyle` 负责统一样式结构

所以后续如果要继续做 UI，推荐思路不是重新发明结构，而是：

- 优先复用现有容器
- 优先复用现有焦点和输入模型
- 需要时新增基础控件或复合控件
- 页面继续按 `Scene + UiScreen` 模式推进

这会是当前仓库里最稳定、最一致、最容易维护的路线。
