# UI ç³»ç»Ÿè¯´æ˜Žä¸Žå·¥ä½œæµ

## 1. å½“å‰ UI ç³»ç»Ÿçš„å®šä½

å½“å‰é¡¹ç›®çš„ UI ç³»ç»Ÿå·²ç»ä¸æ˜¯é›¶æ•£æŽ§ä»¶é›†åˆäº†ï¼Œè€Œæ˜¯ä¸€å¥—å¯ä»¥ç»§ç»­æ‰©å±•çš„ 2D UI æ¡†æž¶ã€‚

å®ƒçŽ°åœ¨å·²ç»è¦†ç›–äº†è¿™äº›èƒ½åŠ›ï¼š

- é¡µé¢å®¹å™¨ä¸Žé¡µé¢åˆ‡æ¢åŠ¨ç”»
- çº¿æ€§å¸ƒå±€ã€æ»šåŠ¨å®¹å™¨ã€åŸºç¡€ç½‘æ ¼å¸ƒå±€
- é€šç”¨ç„¦ç‚¹ä¸Žå¯äº¤äº’æŽ§ä»¶ä½“ç³»
- é”®ç›˜ã€é¼ æ ‡ã€æ‰‹æŸ„å…±ç”¨çš„ UI è¾“å…¥åŠ¨ä½œ
- åŸºç¡€æŽ§ä»¶ã€å¤åˆæŽ§ä»¶ã€è¡¨å•æŽ§ä»¶
- ç»Ÿä¸€ä¸»é¢˜ä¸Žæ ·å¼å…¥å£
- ä¸»èœå•ã€é€‰é¡¹é¡µã€è¡¨å•æ¼”ç¤ºé¡µè¿™ç±»çœŸå®žé¡µé¢

å®ƒè¿˜ä¸æ˜¯å®Œå…¨å°ç®±çš„æœ€ç»ˆ UI æ¡†æž¶ï¼Œä½†å·²ç»è¶³å¤Ÿç¨³å®šï¼Œå¯ä»¥ç»§ç»­æ‰¿è½½èœå•ã€è®¾ç½®é¡µã€å¼¹çª—ã€ç®€å• HUD å’Œè¡¨å•é¡µå¼€å‘ã€‚

## 2. å½“å‰æž¶æž„åˆ†å±‚

### 2.1 `GameObject`

UI ç³»ç»Ÿæœ€åº•å±‚ä»ç„¶å»ºç«‹åœ¨ [game_object.h](/G:/Coding/Projects/Moonline/engine/core/game_object.h) ä¸Šã€‚

`GameObject` è´Ÿè´£ï¼š

- ä¸–ç•Œåæ ‡
- å°ºå¯¸
- `SDL_Rect`
- `on_update`
- `on_render`
- `on_input`
- `on_input_event`

ä¹Ÿå°±æ˜¯è¯´ï¼ŒUI æ²¡æœ‰å¦èµ·ä¸€å¥—å®Œå…¨ç‹¬ç«‹çš„ç”Ÿå‘½å‘¨æœŸï¼Œè€Œæ˜¯å¤ç”¨äº†å¼•æ“Žå¯¹è±¡ç³»ç»Ÿã€‚

### 2.2 `UiElement`

æ–°çš„ UI å…±åŒåŸºç±»æ˜¯ [ui_element.h](/G:/Coding/Projects/Moonline/engine/ui/base/ui_element.h)ã€‚

å®ƒåœ¨ `GameObject` ä¹‹ä¸Šè¡¥äº†è¿™äº› UI å…±æ€§ï¼š

- è‡ªåŠ¨å‘ `UiThemeManager` æ³¨å†Œå’Œæ³¨é”€
- `use_theme`
- `theme_dirty`
- `mark_theme_dirty()`
- `refresh_theme_if_needed()`
- `apply_theme(const UiTheme&)`

è¿™æ„å‘³ç€ç»å¤§å¤šæ•° UI å…ƒç´ éƒ½å¯ä»¥åœ¨ä¸‹ä¸€å¸§è‡ªåŠ¨å“åº”ä¸»é¢˜åˆ‡æ¢ã€‚

### 2.3 `UiControl`

å¯äº¤äº’ UI çš„å…±åŒåŸºç±»æ˜¯ [ui_control.h](/G:/Coding/Projects/Moonline/engine/ui/base/ui_control.h)ã€‚

å®ƒç»§æ‰¿ `UiElement + UiFocusable`ï¼Œç»Ÿä¸€æ”¶å£äº†ï¼š

- `set_enabled / is_enabled`
- `set_focused / is_focused`
- `game_object()`

ç¬¬ä¸€æ‰¹ç›´æŽ¥ç»§æ‰¿ `UiControl` çš„å¶å­æŽ§ä»¶æœ‰ï¼š

- `UiButton`
- `UiSlider`
- `UiToggle`
- `UiTextInput`

### 2.4 `UiFocusable`

[ui_focusable.h](/G:/Coding/Projects/Moonline/engine/ui/base/ui_focusable.h) æ˜¯ç„¦ç‚¹ç³»ç»Ÿçš„æœ€å°å¥‘çº¦ã€‚

å®ƒè¦æ±‚äº¤äº’æŽ§ä»¶è‡³å°‘å®žçŽ°ï¼š

- `set_focused(bool)`
- `is_focused() const`
- `set_enabled(bool)`
- `is_enabled() const`
- `handle_focused_input_event(const InputEvent&)`
- `game_object()`

è¿™å±‚çš„æ„ä¹‰æ˜¯ï¼š

- `UiScreen` ä¸å†åªè®¤è¯†æŒ‰é’®
- ç„¦ç‚¹ç³»ç»Ÿå¯ä»¥ç»Ÿä¸€é©±åŠ¨æŒ‰é’®ã€åˆ—è¡¨ã€å¼€å…³ã€æ»‘æ¡ã€è¾“å…¥æ¡†

## 3. ä¸»è¦å®¹å™¨

### 3.1 `UiLayout`

[ui_layout.h](/G:/Coding/Projects/Moonline/engine/ui/ui_layout.h)

è¿™æ˜¯æœ€æ ¸å¿ƒçš„ UI å®¹å™¨åŸºç±»ï¼Œè´Ÿè´£ï¼š

- child ç®¡ç†
- `Horizontal / Vertical` æŽ’åˆ—
- é”šç‚¹
- å‰¯è½´å¯¹é½
- spacing
- padding
- child margin
- child å°ºå¯¸è¦†ç›–
- `fill_cross_axis`
- å®¹å™¨ transform
- `content_offset`
- auto size

å®ƒä¸æ˜¯é€šç”¨åœºæ™¯æ ‘ï¼Œåªè´Ÿè´£è‡ªå·±å†…éƒ¨ child çš„æ›´æ–°ã€æ¸²æŸ“å’Œè¾“å…¥è½¬å‘ã€‚

### 3.2 `UiPanel`

[ui_panel.h](/G:/Coding/Projects/Moonline/engine/ui/containers/ui_panel.h)

`UiPanel` åœ¨ `UiLayout` ä¸Šå†è¡¥ï¼š

- èƒŒæ™¯è‰²
- è¾¹æ¡†
- èƒŒæ™¯è´´å›¾
- èƒŒæ™¯ alpha
- child è£å‰ª
- `UiPanelThemeRole`

å½“å‰ä¸»é¢˜è§’è‰²æœ‰ï¼š

- `Default`
- `Screen`
- `Dialog`
- `List`

### 3.3 `UiScreen`

[ui_screen.h](/G:/Coding/Projects/Moonline/engine/ui/containers/ui_screen.h)

`UiScreen` æ˜¯é¡µé¢çº§æ ¹å®¹å™¨ï¼Œè´Ÿè´£ï¼š

- open / close
- æ˜¯å¦æŽ¥æ”¶è¾“å…¥
- é¡µé¢è¿‡æ¸¡åŠ¨ç”»
- ç„¦ç‚¹æŽ§ä»¶æ³¨å†Œ
- çº¿æ€§ç„¦ç‚¹å¯¼èˆª

å½“å‰å¯¼èˆªè§„åˆ™ï¼š

- ç«–å‘é¡µé¢é»˜è®¤ `Up / Down`
- æ¨ªå‘é¡µé¢é»˜è®¤ `Left / Right`
- `Confirm` è§¦å‘å½“å‰æŽ§ä»¶ä¸»è¡Œä¸º
- `Cancel` ç”±é¡µé¢æˆ–å¼¹çª—è‡ªå·±å¤„ç†

### 3.4 `UiScrollPanel`

[ui_scroll_panel.h](/G:/Coding/Projects/Moonline/engine/ui/containers/ui_scroll_panel.h)

å®ƒåœ¨ `UiPanel` ä¸Šè¡¥äº†ï¼š

- `scroll_offset`
- `scroll_step`
- æ¨ªå‘/çºµå‘æ»šåŠ¨å¼€å…³
- æ»šåŠ¨èŒƒå›´å¤¹ç´§
- `ensure_child_visible()`
- é¼ æ ‡æ»šè½®æ»šåŠ¨

### 3.5 `UiGridLayout`

[ui_grid_layout.h](/G:/Coding/Projects/Moonline/engine/ui/layout/ui_grid_layout.h)

å½“å‰æ”¯æŒï¼š

- æŒ‡å®šåˆ—æ•°
- è‡ªåŠ¨æ¢è¡Œ
- è¡Œåˆ—é—´è·
- padding
- å•å…ƒæ ¼å†…å¯¹é½

è¿™æ˜¯ç›®å‰æœ€åŸºç¡€çš„äºŒç»´å¸ƒå±€èƒ½åŠ›ã€‚

## 4. åŸºç¡€æŽ§ä»¶

### 4.1 å±•ç¤ºæŽ§ä»¶

- [ui_label.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/label/ui_label.h)
- [ui_image_view.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_image_view.h)
- [ui_progress_bar.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_progress_bar.h)
- [ui_bar.h](/G:/Coding/Projects/Moonline/engine/ui/ui_bar.h)

èŒè´£åˆ†åˆ«æ˜¯ï¼š

- `UiLabel`ï¼šæ–‡æœ¬ã€å­—ä½“ã€é¢œè‰²ã€æ¢è¡Œã€paddingã€å¯¹é½
- `UiImageView`ï¼šè´´å›¾æ˜¾ç¤ºã€è£å‰ªã€ç¼©æ”¾æ¨¡å¼ã€tintã€alpha
- `UiBar`ï¼šçº¯æ•°å€¼æ¡æ¸²æŸ“é€»è¾‘
- `UiProgressBar`ï¼šæŠŠ `UiBar` åŒ…è£…æˆå¯æŒ‚è¿› scene çš„ UI å…ƒç´ 

`UiLabel` çŽ°åœ¨æœ‰ `UiLabelThemeRole`ï¼š

- `Default`
- `Title`
- `Subtitle`
- `Muted`

### 4.2 äº¤äº’æŽ§ä»¶

- [ui_button.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_button.h)
- [ui_text_button.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_text_button.h)
- [ui_slider.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_slider.h)
- [ui_toggle.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_toggle.h)
- [ui_text_input.h](/G:/Coding/Projects/Moonline/engine/ui/widgets/ui_text_input.h)

èŒè´£åˆ†åˆ«æ˜¯ï¼š

- `UiButton`ï¼šæœ€åŸºç¡€æŒ‰é’®ï¼Œæ”¯æŒé¼ æ ‡ç‚¹å‡»å’Œç„¦ç‚¹ç¡®è®¤
- `UiTextButton`ï¼šå¸¦æ–‡å­—æ¸²æŸ“çš„æŒ‰é’®
- `UiSlider`ï¼šè¿žç»­å€¼æŽ§ä»¶ï¼Œæ”¯æŒæ‹–æ‹½å’Œ `Left / Right`
- `UiToggle`ï¼šå¸ƒå°”å¼€å…³ï¼Œæ”¯æŒ `Confirm` å’Œ `Left / Right`
- `UiTextInput`ï¼šå•è¡Œè¾“å…¥æ¡†ï¼Œæ”¯æŒæ–‡æœ¬äº‹ä»¶ã€å…‰æ ‡ç§»åŠ¨ã€åˆ é™¤ã€placeholderã€å¯†ç æ¨¡å¼

`UiButton` æœ‰ `UiButtonThemeRole`ï¼š

- `Default`
- `Primary`
- `Danger`

`UiProgressBar` æœ‰ `UiBarThemeRole`ï¼š

- `Default`
- `Progress`

## 5. å¤åˆæŽ§ä»¶

### 5.1 `UiMenuList`

[ui_menu_list.h](/G:/Coding/Projects/Moonline/engine/ui/composite/ui_menu_list.h)

å®ƒå»ºç«‹åœ¨ `UiScrollPanel + UiTextButton` ä¸Šï¼Œè´Ÿè´£ï¼š

- èœå•é¡¹é›†åˆ
- å½“å‰é€‰ä¸­é¡¹
- ä¸Šä¸‹å¯¼èˆª
- `Confirm` é€‰ä¸­
- è‡ªåŠ¨æ»šåŠ¨åˆ°å¯è§åŒºåŸŸ

### 5.2 `UiOptionList`

[ui_option_list.h](/G:/Coding/Projects/Moonline/engine/ui/composite/ui_option_list.h)

å®ƒå»ºç«‹åœ¨ `UiScrollPanel + UiPanel + UiLabel + (UiSlider / UiToggle)` ä¸Šï¼Œè´Ÿè´£ï¼š

- è®¾ç½®é¡¹åˆ—è¡¨
- è¡Œé€‰ä¸­
- è¡Œå†…æŽ§ä»¶å€¼å˜åŒ–
- ç„¦ç‚¹ä¸Žæ»šåŠ¨è”åŠ¨

### 5.3 `UiDialog`

[ui_dialog.h](/G:/Coding/Projects/Moonline/engine/ui/composite/ui_dialog.h)

å®ƒå»ºç«‹åœ¨ `UiScreen + UiMenuList + UiLabel` ä¸Šï¼Œè´Ÿè´£ï¼š

- æ ‡é¢˜
- æ–‡æ¡ˆ
- æ“ä½œåˆ—è¡¨
- æ¨¡æ€ç¡®è®¤/å–æ¶ˆ

## 6. ä¸»é¢˜ç³»ç»Ÿ

### 6.1 å…¥å£

ä¸»é¢˜ç³»ç»Ÿæ ¸å¿ƒæ–‡ä»¶æ˜¯ï¼š

- [ui_theme.h](/G:/Coding/Projects/Moonline/engine/ui/style/ui_theme.h)
- [ui_theme_manager.h](/G:/Coding/Projects/Moonline/engine/ui/style/ui_theme_manager.h)

### 6.2 å·¥ä½œæ–¹å¼

`UiThemeManager` ç»´æŠ¤å½“å‰å…¨å±€ä¸»é¢˜ã€‚

å½“è°ƒç”¨ï¼š

```cpp
UiThemeManager::instance().set_theme(new_theme);
```

æ—¶ï¼Œä¸ä¼šç«‹åˆ»åŒæ­¥è°ƒç”¨æ‰€æœ‰æŽ§ä»¶çš„è™šå‡½æ•°ï¼Œè€Œæ˜¯ï¼š

1. æ ‡è®°æ‰€æœ‰å·²æ³¨å†Œ `UiElement` ä¸º `theme_dirty`
2. è¿™äº›å…ƒç´ åœ¨ä¸‹ä¸€æ¬¡ `on_update / on_render / on_input` å…¥å£é‡Œæ‰§è¡Œ `refresh_theme_if_needed()`
3. æœ€ç»ˆè¿›å…¥å„è‡ªçš„ `apply_theme(const UiTheme&)`

è¿™æ ·åšçš„å¥½å¤„æ˜¯ï¼š

- å¯ä»¥è¿è¡Œæ—¶åˆ‡ä¸»é¢˜
- ä¸å®¹æ˜“åœ¨å¯¹è±¡åŠåˆå§‹åŒ–çŠ¶æ€ä¸‹è§¦å‘æ ·å¼åº”ç”¨

### 6.3 é»˜è®¤ä¸»é¢˜

é»˜è®¤ä¸»é¢˜å·¥åŽ‚æ˜¯ï¼š

- [ui_theme.cpp](/G:/Coding/Projects/Moonline/engine/ui/style/ui_theme.cpp)

å½“å‰é»˜è®¤ä¸»é¢˜ä»¥ loading bar çš„è§†è§‰æ–¹å‘ä¸ºä¸­å¿ƒï¼š

- æ·±æµ·è“èƒŒæ™¯
- å†·ç™½å¡«å……
- é’è“è¾¹æ¡†å’Œé«˜äº®
- ä½Žé¥±å’Œå†·è‰²æ¬¡çº§æ–‡å­—

ä¹Ÿå°±æ˜¯è¯´ï¼Œæ•´å¥—é»˜è®¤ UI ä¼šæ¯”è¾ƒæŽ¥è¿‘å½“å‰åŠ è½½æ¡çš„æ°”è´¨ï¼Œè€Œä¸æ˜¯é€šç”¨ç°ç™½èœå•ã€‚

## 7. è¾“å…¥æµ

è¾“å…¥é“¾è·¯å¤§è‡´æ˜¯ï¼š

1. SDL äº‹ä»¶è¿›å…¥è¾“å…¥ç³»ç»Ÿ
2. è¾“å…¥ç³»ç»Ÿæ•´ç†æˆ `InputSnapshot + InputEvent`
3. scene æŠŠè¾“å…¥åˆ†å‘ç»™é¡µé¢æ ¹å¯¹è±¡
4. `UiScreen` å…ˆå¤„ç†é¡µé¢çº§ç„¦ç‚¹å¯¼èˆª
5. å½“å‰èšç„¦çš„ `UiFocusable` å¤„ç†è‡ªå·±çš„è¡Œä¸º

ä¾‹å¦‚ï¼š

- `Up / Down` åœ¨ `UiScreen` é‡Œåˆ‡æ¢ç„¦ç‚¹
- `Confirm` ä¼šè½åˆ° `UiButton / UiMenuList / UiToggle`
- `Left / Right` ä¼šè½åˆ° `UiSlider / UiToggle / UiOptionList`
- `TextInput` äº‹ä»¶ä¼šè½åˆ° `UiTextInput`

## 8. å½“å‰é¡µé¢è½åœ°

### 8.1 ä¸»èœå•

- [main_menu_scene.h](/G:/Coding/Projects/Moonline/gameplay/scene/main_menu_scene.h)
- [main_menu_scene.cpp](/G:/Coding/Projects/Moonline/gameplay/scene/main_menu_scene.cpp)

ä½¿ç”¨ï¼š

- `UiScreen`
- `UiLabel`
- `UiMenuList`
- `UiScrollBar`
- `UiDialog`

### 8.2 é€‰é¡¹é¡µ

- [options_scene.h](/G:/Coding/Projects/Moonline/gameplay/scene/options_scene.h)
- [options_scene.cpp](/G:/Coding/Projects/Moonline/gameplay/scene/options_scene.cpp)

ä½¿ç”¨ï¼š

- `UiScreen`
- `UiLabel`
- `UiOptionList`
- `UiScrollBar`

### 8.3 è¡¨å•æ¼”ç¤ºé¡µ

- [ui_forms_demo_scene.h](/G:/Coding/Projects/Moonline/gameplay/scene/ui_forms_demo_scene.h)
- [ui_forms_demo_scene.cpp](/G:/Coding/Projects/Moonline/gameplay/scene/ui_forms_demo_scene.cpp)

ä½¿ç”¨ï¼š

- `UiScreen`
- `UiGridLayout`
- `UiLabel`
- `UiTextInput`
- `UiToggle`
- `UiSlider`
- `UiTextButton`

## 9. å¦‚æžœè¦æ–°å¢ž UIï¼ŒæŽ¨èå·¥ä½œæµç¨‹

### 9.1 æ–°å¢žä¸€ä¸ªç®€å•é¡µé¢

æŽ¨èæ­¥éª¤ï¼š

1. æ–°å»ºä¸€ä¸ª scene
2. åœ¨ scene é‡ŒæŒæœ‰ä¸€ä¸ª `std::shared_ptr<UiScreen>`
3. å†æŒæœ‰è¿™ä¸ªé¡µé¢æ‰€éœ€çš„æŽ§ä»¶
4. åœ¨ `ensure_ui()` é‡Œæ‡’åˆ›å»º
5. åœ¨ `reset()` é‡Œï¼š
   - `reset()` æŽ§ä»¶
   - è®¾ç½®æ–‡æœ¬ã€å°ºå¯¸ã€å›žè°ƒ
   - è®¾ç½® theme role
   - æŒ‚è¿› layout
   - æ³¨å†Œå¯èšç„¦æŽ§ä»¶
6. æœ€åŽ `add_object(_screen)`

### 9.2 æ–°å¢žä¸€ä¸ªåŸºç¡€æŽ§ä»¶

å¦‚æžœå®ƒæ˜¯å¯äº¤äº’æŽ§ä»¶ï¼š

1. ç»§æ‰¿ `UiControl`
2. å®žçŽ° `handle_focused_input_event`
3. åœ¨ `on_render / on_update` å…¥å£è°ƒç”¨ `refresh_theme_if_needed()`
4. å®žçŽ° `apply_theme(const UiTheme&)`

å¦‚æžœå®ƒåªæ˜¯å±•ç¤ºæŽ§ä»¶ï¼š

1. ç»§æ‰¿ `UiElement`
2. åœ¨ç”Ÿå‘½å‘¨æœŸå…¥å£è°ƒç”¨ `refresh_theme_if_needed()`
3. å®žçŽ° `apply_theme(const UiTheme&)`

### 9.3 æ–°å¢žä¸€ä¸ªå¤åˆæŽ§ä»¶

æŽ¨èåšæ³•ï¼š

1. å…ˆé€‰ä¸€ä¸ªåˆé€‚å®¹å™¨åŸºç±»
   - `UiPanel`
   - `UiScrollPanel`
   - `UiScreen`
2. å†å†³å®šå®ƒæ˜¯å¦è¿˜éœ€è¦å®žçŽ° `UiFocusable`
3. å†…éƒ¨ç»„åˆå¶å­æŽ§ä»¶ï¼Œè€Œä¸æ˜¯å †å¤šç»§æ‰¿
4. éœ€è¦ä¸»é¢˜ç»Ÿä¸€æ—¶ï¼Œåœ¨å¤åˆæŽ§ä»¶é‡Œè¦†å†™ `apply_theme`

### 9.4 åšä¸»é¢˜åˆ‡æ¢

æœ€ç›´æŽ¥çš„æ–¹å¼ï¼š

```cpp
UiTheme theme = make_loading_blue_theme();
theme._primary_button._idle_color = SDL_Color{ 34, 84, 120, 255 };
UiThemeManager::instance().set_theme(theme);
```

å¦‚æžœæŸä¸ªæŽ§ä»¶ä¸æƒ³è·Ÿç€å…¨å±€ä¸»é¢˜èµ°ï¼š

```cpp
element->set_use_theme(false);
```

ç„¶åŽå†æ‰‹åŠ¨è°ƒç”¨ `UiStyle::apply_*()` æˆ–è‡ªå·±çš„ setter å³å¯ã€‚

## 10. å½“å‰å‘½åçº¦å®š

è¿™è½®é‡æž„ä¹‹åŽï¼ŒæŽ¨èæŠŠ UI ç±»åž‹ç»Ÿä¸€çœ‹æˆè¿™å‡ æ¡£ï¼š

- é¡µé¢ä¸Žå®¹å™¨ï¼š`UiScreen / UiPanel / UiScrollPanel / UiGridLayout / UiLayout`
- å±•ç¤ºæŽ§ä»¶ï¼š`UiLabel / UiImageView / UiProgressBar`
- äº¤äº’æŽ§ä»¶ï¼š`UiButton / UiTextButton / UiSlider / UiToggle / UiTextInput`
- å¤åˆæŽ§ä»¶ï¼š`UiMenuList / UiOptionList / UiDialog`

`UiBar` çŽ°åœ¨ä¹Ÿçº³å…¥ç»Ÿä¸€ UI å‘½åï¼Œä½†èŒè´£ä»ç„¶ä¿æŒä¸ºçº¯æ•°å€¼æ¡æ¸²æŸ“é€»è¾‘å¯¹è±¡ï¼Œä¸ç›´æŽ¥æŒ‚åœºæ™¯ã€‚

## 11. å½“å‰è¿˜æ²¡å®Œå…¨è§£å†³çš„ç‚¹

è™½ç„¶åŸºç¡€å±‚å·²ç»æ¯”è¾ƒå®Œæ•´ï¼Œä½†è¿˜æ²¡åˆ°å®Œå…¨ä¸ç”¨å†è¡¥çš„ç¨‹åº¦ã€‚

åŽç»­ä»ç„¶å€¼å¾—ç»§ç»­è¡¥çš„æ–¹å‘æœ‰ï¼š

- æ›´å¤æ‚çš„ç©ºé—´å¯¼èˆª
- æ›´å®Œæ•´çš„ modal/focus scope
- æ›´æˆç†Ÿçš„ theme é…ç½®æ¥æº
- ä¸‹æ‹‰æ¡†ã€tabã€tooltip ç­‰é«˜çº§æŽ§ä»¶
- æ›´å¤æ‚çš„ HUD ä¸“ç”¨é¡µé¢ç»„åˆ

## 12. ä¸€å¥è¯æ€»ç»“

å½“å‰ UI ç³»ç»Ÿçš„æ ¸å¿ƒå·¥ä½œæµå¯ä»¥æ¦‚æ‹¬æˆï¼š

`Scene` è´Ÿè´£é¡µé¢ç»„ç»‡ï¼Œ`UiScreen` è´Ÿè´£ç„¦ç‚¹å’Œé¡µé¢è¡Œä¸ºï¼Œ`UiElement / UiControl` è´Ÿè´£å…±äº« UI åŸºç¡€èƒ½åŠ›ï¼Œå¶å­æŽ§ä»¶è´Ÿè´£å…·ä½“è¡¨çŽ°ï¼Œ`UiThemeManager` è´Ÿè´£ç»Ÿä¸€è§†è§‰é£Žæ ¼ã€‚
