# UI 系统说明与工作流

## 1. 当前 UI 系统的定位

当前项目的 UI 系统已经不是零散控件集合了，而是一套可以继续扩展的 2D UI 框架。

它现在已经覆盖了这些能力：

- 页面容器与页面切换动画
- 线性布局、滚动容器、基础网格布局
- 通用焦点与可交互控件体系
- 键盘、鼠标、手柄共用的 UI 输入动作
- 基础控件、复合控件、表单控件
- 统一主题与样式入口
- 主菜单、选项页、表单演示页这类真实页面

它还不是完全封箱的最终 UI 框架，但已经足够稳定，可以继续承载菜单、设置页、弹窗、简单 HUD 和表单页开发。

## 2. 当前架构分层

### 2.1 `GameObject`

UI 系统最底层仍然建立在 [game_object.h](/G:/Coding/Projects/Moonline/engine/core/game_object.h) 上。

`GameObject` 负责：

- 世界坐标
- 尺寸
- `SDL_Rect`
- `on_update`
- `on_render`
- `on_input`
- `on_input_event`

也就是说，UI 没有另起一套完全独立的生命周期，而是复用了引擎对象系统。

### 2.2 `UiElement`

新的 UI 共同基类是 [ui_element.h](/G:/Coding/Projects/Moonline/engine/ui/base/ui_element.h)。

它在 `GameObject` 之上补了这些 UI 共性：

- 自动向 `UiThemeManager` 注册和注销
- `use_theme`
- `theme_dirty`
- `mark_theme_dirty()`
- `refresh_theme_if_needed()`
- `apply_theme(const UiTheme&)`

这意味着绝大多数 UI 元素都可以在下一帧自动响应主题切换。

### 2.3 `UiControl`

可交互 UI 的共同基类是 [ui_control.h](/G:/Coding/Projects/Moonline/engine/ui/base/ui_control.h)。

它继承 `UiElement + UiFocusable`，统一收口了：

- `set_enabled / is_enabled`
- `set_focused / is_focused`
- `game_object()`

第一批直接继承 `UiControl` 的叶子控件有：

- `UiButton`
- `UiSlider`
- `UiToggle`
- `UiTextInput`

### 2.4 `UiFocusable`

[ui_focusable.h](/G:/Coding/Projects/Moonline/engine/ui/base/ui_focusable.h) 是焦点系统的最小契约。

它要求交互控件至少实现：

- `set_focused(bool)`
- `is_focused() const`
- `set_enabled(bool)`
- `is_enabled() const`
- `handle_focused_input_event(const InputEvent&)`
- `game_object()`

这层的意义是：

- `UiScreen` 不再只认识按钮
- 焦点系统可以统一驱动按钮、列表、开关、滑条、输入框

## 3. 主要容器

### 3.1 `UILayout`

[ui_layout.h](/G:/Coding/Projects/Moonline/engine/ui/ui_layout.h)

这是最核心的 UI 容器基类，负责：

- child 管理
- `Horizontal / Vertical` 排列
- 锚点
- 副轴对齐
- spacing
- padding
- child margin
- child 尺寸覆盖
- `fill_cross_axis`
- 容器 transform
- `content_offset`
- auto size

它不是通用场景树，只负责自己内部 child 的更新、渲染和输入转发。

### 3.2 `UiPanel`

[ui_panel.h](/G:/Coding/Projects/Moonline/engine/ui/containers/ui_panel.h)

`UiPanel` 在 `UILayout` 上再补：

- 背景色
- 边框
- 背景贴图
- 背景 alpha
- child 裁剪
- `UiPanelThemeRole`

当前主题角色有：

- `Default`
- `Screen`
- `Dialog`
- `List`

### 3.3 `UiScreen`

[ui_screen.h](/G:/Coding/Projects/Moonline/engine/ui/containers/ui_screen.h)

`UiScreen` 是页面级根容器，负责：

- open / close
- 是否接收输入
- 页面过渡动画
- 焦点控件注册
- 线性焦点导航

当前导航规则：

- 竖向页面默认 `Up / Down`
- 横向页面默认 `Left / Right`
- `Confirm` 触发当前控件主行为
- `Cancel` 由页面或弹窗自己处理

### 3.4 `UiScrollPanel`

[ui_scroll_panel.h](/G:/Coding/Projects/Moonline/engine/ui/containers/ui_scroll_panel.h)

它在 `UiPanel` 上补了：

- `scroll_offset`
- `scroll_step`
- 横向/纵向滚动开关
- 滚动范围夹紧
- `ensure_child_visible()`
- 鼠标滚轮滚动

### 3.5 `UiGridLayout`

[ui_grid_layout.h](/G:/Coding/Projects/Moonline/engine/ui/layout/ui_grid_layout.h)

当前支持：

- 指定列数
- 自动换行
- 行列间距
- padding
- 单元格内对齐

这是目前最基础的二维布局能力。

## 4. 基础控件

### 4.1 展示控件

- [ui_label.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_label.h)
- [ui_image_view.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_image_view.h)
- [ui_progress_bar.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_progress_bar.h)
- [bar.h](/G:/Coding/Projects/Moonline/engine/ui/bar.h)

职责分别是：

- `UiLabel`：文本、字体、颜色、换行、padding、对齐
- `UiImageView`：贴图显示、裁剪、缩放模式、tint、alpha
- `Bar`：纯数值条渲染逻辑
- `UiProgressBar`：把 `Bar` 包装成可挂进 scene 的 UI 元素

`UiLabel` 现在有 `UiLabelThemeRole`：

- `Default`
- `Title`
- `Subtitle`
- `Muted`

### 4.2 交互控件

- [ui_button.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_button.h)
- [ui_text_button.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_text_button.h)
- [ui_slider.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_slider.h)
- [ui_toggle.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_toggle.h)
- [ui_text_input.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_text_input.h)

职责分别是：

- `UiButton`：最基础按钮，支持鼠标点击和焦点确认
- `UiTextButton`：带文字渲染的按钮
- `UiSlider`：连续值控件，支持拖拽和 `Left / Right`
- `UiToggle`：布尔开关，支持 `Confirm` 和 `Left / Right`
- `UiTextInput`：单行输入框，支持文本事件、光标移动、删除、placeholder、密码模式

`UiButton` 有 `UiButtonThemeRole`：

- `Default`
- `Primary`
- `Danger`

`UiProgressBar` 有 `UiBarThemeRole`：

- `Default`
- `Progress`

## 5. 复合控件

### 5.1 `UiMenuList`

[ui_menu_list.h](/G:/Coding/Projects/Moonline/engine/ui/composite/ui_menu_list.h)

它建立在 `UiScrollPanel + UiTextButton` 上，负责：

- 菜单项集合
- 当前选中项
- 上下导航
- `Confirm` 选中
- 自动滚动到可见区域

### 5.2 `UiOptionList`

[ui_option_list.h](/G:/Coding/Projects/Moonline/engine/ui/composite/ui_option_list.h)

它建立在 `UiScrollPanel + UiPanel + UiLabel + (UiSlider / UiToggle)` 上，负责：

- 设置项列表
- 行选中
- 行内控件值变化
- 焦点与滚动联动

### 5.3 `UiDialog`

[ui_dialog.h](/G:/Coding/Projects/Moonline/engine/ui/composite/ui_dialog.h)

它建立在 `UiScreen + UiMenuList + UiLabel` 上，负责：

- 标题
- 文案
- 操作列表
- 模态确认/取消

## 6. 主题系统

### 6.1 入口

主题系统核心文件是：

- [ui_theme.h](/G:/Coding/Projects/Moonline/engine/ui/style/ui_theme.h)
- [ui_theme_manager.h](/G:/Coding/Projects/Moonline/engine/ui/style/ui_theme_manager.h)

### 6.2 工作方式

`UiThemeManager` 维护当前全局主题。

当调用：

```cpp
UiThemeManager::instance().set_theme(new_theme);
```

时，不会立刻同步调用所有控件的虚函数，而是：

1. 标记所有已注册 `UiElement` 为 `theme_dirty`
2. 这些元素在下一次 `on_update / on_render / on_input` 入口里执行 `refresh_theme_if_needed()`
3. 最终进入各自的 `apply_theme(const UiTheme&)`

这样做的好处是：

- 可以运行时切主题
- 不容易在对象半初始化状态下触发样式应用

### 6.3 默认主题

默认主题工厂是：

- [ui_theme.cpp](/G:/Coding/Projects/Moonline/engine/ui/style/ui_theme.cpp)

当前默认主题以 loading bar 的视觉方向为中心：

- 深海蓝背景
- 冷白填充
- 青蓝边框和高亮
- 低饱和冷色次级文字

也就是说，整套默认 UI 会比较接近当前加载条的气质，而不是通用灰白菜单。

## 7. 输入流

输入链路大致是：

1. SDL 事件进入输入系统
2. 输入系统整理成 `InputSnapshot + InputEvent`
3. scene 把输入分发给页面根对象
4. `UiScreen` 先处理页面级焦点导航
5. 当前聚焦的 `UiFocusable` 处理自己的行为

例如：

- `Up / Down` 在 `UiScreen` 里切换焦点
- `Confirm` 会落到 `UiButton / UiMenuList / UiToggle`
- `Left / Right` 会落到 `UiSlider / UiToggle / UiOptionList`
- `TextInput` 事件会落到 `UiTextInput`

## 8. 当前页面落地

### 8.1 主菜单

- [main_menu_scene.h](/G:/Coding/Projects/Moonline/gameplay/scene/main_menu_scene.h)
- [main_menu_scene.cpp](/G:/Coding/Projects/Moonline/gameplay/scene/main_menu_scene.cpp)

使用：

- `UiScreen`
- `UiLabel`
- `UiMenuList`
- `UiScrollBar`
- `UiDialog`

### 8.2 选项页

- [options_scene.h](/G:/Coding/Projects/Moonline/gameplay/scene/options_scene.h)
- [options_scene.cpp](/G:/Coding/Projects/Moonline/gameplay/scene/options_scene.cpp)

使用：

- `UiScreen`
- `UiLabel`
- `UiOptionList`
- `UiScrollBar`

### 8.3 表单演示页

- [ui_forms_demo_scene.h](/G:/Coding/Projects/Moonline/gameplay/scene/ui_forms_demo_scene.h)
- [ui_forms_demo_scene.cpp](/G:/Coding/Projects/Moonline/gameplay/scene/ui_forms_demo_scene.cpp)

使用：

- `UiScreen`
- `UiGridLayout`
- `UiLabel`
- `UiTextInput`
- `UiToggle`
- `UiSlider`
- `UiTextButton`

## 9. 如果要新增 UI，推荐工作流程

### 9.1 新增一个简单页面

推荐步骤：

1. 新建一个 scene
2. 在 scene 里持有一个 `std::shared_ptr<UiScreen>`
3. 再持有这个页面所需的控件
4. 在 `ensure_ui()` 里懒创建
5. 在 `reset()` 里：
   - `reset()` 控件
   - 设置文本、尺寸、回调
   - 设置 theme role
   - 挂进 layout
   - 注册可聚焦控件
6. 最后 `add_object(_screen)`

### 9.2 新增一个基础控件

如果它是可交互控件：

1. 继承 `UiControl`
2. 实现 `handle_focused_input_event`
3. 在 `on_render / on_update` 入口调用 `refresh_theme_if_needed()`
4. 实现 `apply_theme(const UiTheme&)`

如果它只是展示控件：

1. 继承 `UiElement`
2. 在生命周期入口调用 `refresh_theme_if_needed()`
3. 实现 `apply_theme(const UiTheme&)`

### 9.3 新增一个复合控件

推荐做法：

1. 先选一个合适容器基类
   - `UiPanel`
   - `UiScrollPanel`
   - `UiScreen`
2. 再决定它是否还需要实现 `UiFocusable`
3. 内部组合叶子控件，而不是堆多继承
4. 需要主题统一时，在复合控件里覆写 `apply_theme`

### 9.4 做主题切换

最直接的方式：

```cpp
UiTheme theme = make_loading_blue_theme();
theme._primary_button._idle_color = SDL_Color{ 34, 84, 120, 255 };
UiThemeManager::instance().set_theme(theme);
```

如果某个控件不想跟着全局主题走：

```cpp
element->set_use_theme(false);
```

然后再手动调用 `UiStyle::apply_*()` 或自己的 setter 即可。

## 10. 当前命名约定

这轮重构之后，推荐把 UI 类型统一看成这几档：

- 页面与容器：`UiScreen / UiPanel / UiScrollPanel / UiGridLayout / UILayout`
- 展示控件：`UiLabel / UiImageView / UiProgressBar`
- 交互控件：`UiButton / UiTextButton / UiSlider / UiToggle / UiTextInput`
- 复合控件：`UiMenuList / UiOptionList / UiDialog`

`Bar` 保持不带 `Ui` 前缀，因为它现在是纯渲染逻辑对象，不是直接挂场景的 UI 元素。

## 11. 当前还没完全解决的点

虽然基础层已经比较完整，但还没到完全不用再补的程度。

后续仍然值得继续补的方向有：

- 更复杂的空间导航
- 更完整的 modal/focus scope
- 更成熟的 theme 配置来源
- 下拉框、tab、tooltip 等高级控件
- 更复杂的 HUD 专用页面组合

## 12. 一句话总结

当前 UI 系统的核心工作流可以概括成：

`Scene` 负责页面组织，`UiScreen` 负责焦点和页面行为，`UiElement / UiControl` 负责共享 UI 基础能力，叶子控件负责具体表现，`UiThemeManager` 负责统一视觉风格。
