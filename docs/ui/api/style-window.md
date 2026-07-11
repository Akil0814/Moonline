# API：样式、主题与窗口

本页覆盖 `style/` 与 `window/` 的公开 API。样式结构均为值类型；只有显式设为
`std::optional` 的覆盖字段才会替换 base style 中对应字段。

## 通用样式类型

| API | 含义 |
| --- | --- |
| `style/ui_interaction_style.h` — `UiEnabledDisabledColors` | `enabled` 和 `disabled` 两色；用于仅区分可用性的文字/标记。 |
| `UiEnabledDisabledColorsOverrides` | enabled/disabled 的可选覆盖；`empty` 与 `apply_ui_style_overrides` 判断/合并。 |
| `UiInteractiveColors` | idle、focused、pushed、disabled 四态色。 |
| `UiInteractiveColorsOverrides` | 上述四态的稀疏覆盖；同样提供 `empty`/`apply_ui_style_overrides`。 |
| `UiChromeStyle` | 背景、边框、圆角、是否绘制背景/边框的通用外框样式。 |
| `UiChromeStyleOverrides` | chrome 字段的可选覆盖；用于 Button、Checkbox 等交互表面。 |
| `style/ui_style.h` — `UiStyleOverrideTraits<T>` | 类型专用的“覆盖是否为空、如何应用”契约；为新样式类型提供特化。 |
| `UiStyleState<T>` | 保存 base style 与 overrides。`set_base_style` 不丢失覆盖；`set_overrides` 替换覆盖；`style` 返回合并结果；`base_style`/`overrides` 查询；`has_overrides`/`clear_overrides` 管理覆盖。 |
| `style/ui_visual_styles.h` | `UiLabelStyle`、`UiTextBlockStyle`、`UiDropdownStyle`、`UiNumberStyle`、`UiBarStyle`、`UiPanelStyle`、`UiChromeContainerStyle`、`UiWindowStyle`、`UiDialogStyle` 是各类完整样式；同名 `Overrides` 均是字段级可选覆盖。 |
| `UiOverlayOptionsOverrides` | 对 overlay 的 open/modal/关闭策略、placement、transition、fallback_size、order 的可选覆盖。 |
| `style/ui_style_defaults.h` — `UiStyleDefaults` | 静态默认样式工厂；按组件返回无主题时的安全视觉基线。 |
| `style/ui_palette.h` — `UiPalette` | 命名色板；内建主题构造与自定义主题可复用。 |

## 视觉角色和主题

| API | 含义 |
| --- | --- |
| `style/ui_visual_roles.h` — `UiPanelVisualRole` | `Default`、`Dialog`、`Tooltip` 等面板语义。 |
| `UiLabelVisualRole`、`UiTextBlockVisualRole` | 普通、标题、说明等文字语义。 |
| `UiButtonVisualRole` | `Default`、`Primary`、`Danger`、`Tab` 等按钮语义。 |
| `UiBarVisualRole`、`UiDialogVisualRole`、`UiDropdownVisualRole` | 进度、对话框、下拉的主题选择键。具体枚举值应按语义而非颜色选择。 |
| `style/ui_theme.h` — `UiBuiltinTheme` | `BlueGlassMoon`、`ElysiaLight`、`ElysiaDark`、`EvangelionUnit00/01/02`、`QuietSlate`。 |
| `Ui*ThemeColors` | `UiChromeThemeColors` 到 `UiScrollContainerThemeColors`：每个组件族的主题颜色包；它们构成 `UiTheme` 的字段。 |
| `UiTheme` | 完整主题；`label`、`button`、`panel`、`dialog`、`dropdown` 等方法按 visual role 返回对应完整 style。 |
| `make_builtin_theme(UiBuiltinTheme)` | 构造指定内建主题的值对象。 |
| `style/ui_theme_style_resolver.h` — `UiThemeStyleTraversal` | `CurrentOnly` 或 `Subtree`，控制解析/刷新范围。 |
| `UiThemeStyleResolution` | 一次解析结果：目标 element、是否有可应用样式及遍历信息。 |
| `UiThemeStyleResolver::resolve/apply` | 根据 `UiTheme`、元素实际类型和 visual role 解析并应用基础样式；`apply_subtree` 递归处理子树。 |
| `style/ui_theme_manager.h` — `UiThemeRegistration` | 根注册的 RAII 令牌；不可复制、可移动。`reset` 立即解绑，`registered` 查询是否仍有效。 |
| `UiThemeManager::register_root/unregister_root` | 关联/解除一棵 `UiChildHost` 子树与 manager。保留返回 registration，或显式 unregister。 |
| `set_theme/current_builtin_theme/current_theme/reapply_theme` | 切换内建主题、查询主题、重新应用当前主题。 |
| `refresh_element/attach_and_apply_subtree/detach_subtree/on_host_destroying` | 供 host 生命周期和高级集成刷新单元素、接入/分离子树并清理销毁关系；普通场景主要使用 `register_root` 与 `set_theme`。 |

## Overlay 与 popup 协议

| API | 含义 |
| --- | --- |
| `window/ui_overlay.h` — `UiOverlayWindowClient` | 实现 `on_overlay_window_detached(window)` 的借用窗口客户端协议；窗口释放登记前通知对象清除本地借用状态。 |
| `UiOverlayPlacement` | `Center`、`LeftDrawer`、`RightDrawer`、`TopSheet`、`BottomSheet`。 |
| `UiOverlayTransition` | `None` 或 `Slide`。 |
| `UiOverlayOptions` | `open=true`、`modal=true`、关闭策略、placement、transition、`fallback_size=(360,220)`、`order=1000`；open 状态由窗口登记项管理而非元素寿命。 |
| `window/ui_transient_popup.h` — `UiTransientPopup` | 非模态浮层协议：提供 owner、开关状态、命中测试、关闭、窗口解绑、事件路由和 popup 命令提交。实现者拥有 popup 内容，`UiWindow` 只协调。 |

## `UiWindow`

`UiWindow` 是 `UiChildHost` 根节点，样式使用 `UiWindowStyle`/`UiWindowStyleOverrides`。它不拥有
通过 register 方法传入的 scope/popup/tooltip 指针；它们必须是仍属于该窗口子树的存活对象。

| API | 含义 |
| --- | --- |
| 构造器与 `reset` | 使用 rect、position/size 或中心标签创建；reset 解除窗口登记并清理 child 状态。 |
| `set_base_style/style/overrides/has_style_overrides/clear_style_overrides` | 管理窗口视觉。 |
| `set_hover_focus_enabled/hover_focus_enabled` | 设置鼠标悬停是否可移动 scope 焦点。 |
| `set_on_cancel` | 没有更高优先级表面消费 Cancel 时调用的窗口级回调。 |
| `register_focus_scope/unregister_focus_scope` | 登记/移除一个 focus scope；可同时给 `UiFocusScopeNeighbors`。 |
| `set_scope_neighbors` | 替换已登记 scope 的四向窗口级导航链接。 |
| `set_focused_scope/focused_scope/focus_first_available_scope/focus_input_device` | 显式设置、读取或选取首个可用 scope；读取最近驱动焦点的设备。 |
| `register_overlay/unregister_overlay` | 把窗口拥有的 child 登记为 overlay，可给 `UiOverlayOptions`。 |
| `set_overlay_open/is_overlay_open/overlay_options` | 开闭某登记 overlay、查询状态和可变 options。关闭时尝试恢复先前 scope。 |
| `register_transient_popup/unregister_transient_popup/activate_transient_popup` | 登记、解除或激活 `UiTransientPopup`；激活一个会关闭先前活动 popup。 |
| `register_tooltip/unregister_tooltip` | 登记/解除被动 tooltip。 |
| `is_tooltip_pointer_blocked/blocks_background_tooltips` | 供 tooltip 判定活动 popup 是否遮挡指针或导航提示。 |
| `content_bounds` | 返回扣除窗口 padding 后的布局区域。 |
| `update/on_ui_input_frame/on_ui_input_event/submit_ui_render_commands` | 维护失效登记、布局和焦点，并按 overlay → transient popup → 普通 child → tooltip 的层级处理输入/绘制。 |

## 样式使用建议

先用 role 表达“主按钮”“对话框”等语义，再通过主题决定视觉；仅对产品确实必须固定的
字段写 override。直接在 child 上设置 base style 会在主题刷新时被 resolver 重置，除非该 child
未被主题树管理或其字段有明确 overrides。`tests/ui_lifecycle_tests.cpp` 覆盖了字段级样式层叠、
主题树传播和复合控件内部标签不丢失基础样式的行为。
