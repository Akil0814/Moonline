#include "ui_container_test_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../engine/resources/resource_manager.h"
#include "../../engine/ui/composites/ui_confirmation_dialog.h"
#include "../../engine/ui/composites/ui_dialog.h"
#include "../../engine/ui/composites/ui_dropdown.h"
#include "../../engine/ui/composites/ui_labeled_checkbox.h"
#include "../../engine/ui/composites/ui_labeled_radio_button.h"
#include "../../engine/ui/composites/ui_tab_container.h"
#include "../../engine/ui/composites/ui_tooltip.h"
#include "../../engine/ui/containers/ui_button_group.h"
#include "../../engine/ui/containers/ui_chrome_container.h"
#include "../../engine/ui/containers/ui_grid_container.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_radio_group.h"
#include "../../engine/ui/containers/ui_scroll_container.h"
#include "../../engine/ui/widgets/image/ui_animation.h"
#include "../../engine/ui/widgets/image/ui_blink_image.h"
#include "../../engine/ui/widgets/image/ui_fade_image.h"
#include "../../engine/ui/widgets/image/ui_image.h"
#include "../../engine/ui/widgets/image/ui_pulse_image.h"
#include "../../engine/ui/widgets/label/ui_blink_label.h"
#include "../../engine/ui/widgets/label/ui_fade_label.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/widgets/label/ui_pulse_label.h"
#include "../../engine/ui/widgets/number/ui_number.h"
#include "../../engine/ui/widgets/text/ui_text_block.h"
#include "../../engine/ui/widgets/ui_bar.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/ui_checkbox.h"
#include "../../engine/ui/widgets/ui_drag_handle.h"
#include "../../engine/ui/widgets/ui_radio_button.h"
#include "../../engine/ui/widgets/ui_slider.h"
#include "../../engine/ui/widgets/ui_text_input.h"
#include "../../engine/ui/window/ui_window.h"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace arcneco::scene
{
namespace
{
using namespace elysia::ui;

UiLayoutChildOptions at(float x,float y,float width,float height)
{
    return UiLayoutChildOptions{
        ._anchor = UiLayoutAnchor::TopLeft,
        ._margin = UiLayoutMargin{ x,y,0.0f,0.0f },
        ._cross_align = UiLayoutAlign::Start,
        ._size_override = elysia::core::Vector2(width,height),
        ._use_custom_cross_align = false,
        ._fill_cross_axis = false,
        ._use_size_override = true
    };
}

std::unique_ptr<UiButton> button(const char* key)
{
    return std::make_unique<UiButton>(elysia::core::Rect{ 0,0,240,40 },UiButtonConfig{ .content = ui_text_key(key) },0);
}

std::unique_ptr<UiScrollContainer> page_scroll(UiListContainer*& content)
{
    auto page = std::make_unique<UiScrollContainer>(elysia::core::Rect{ 0,0,900,390 });
    page->set_scroll_axis(UiScrollAxis::Vertical);
    page->set_scrollbar_visibility(UiScrollBarVisibility::Auto);
    page->set_scroll_step(elysia::core::Vector2(0.0f,36.0f));
    auto list = std::make_unique<UiListContainer>(elysia::core::Rect{ 0,0,870,0 });
    list->set_padding(UiLayoutPadding{ 12,12,12,12 });
    list->set_item_spacing(12.0f);
    list->set_cross_align(UiLayoutAlign::Start);
    content = list.get();
    page->set_content(std::move(list));
    return page;
}

UiListContainer* add_section(UiListContainer& page,const char* title,const char* description)
{
    auto chrome = std::make_unique<UiChromeContainer>(elysia::core::Rect{ 0,0,840,0 });
    chrome->set_header_height(42.0f);
    auto heading = std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,420,32 },0,ui_text_key(title));
    heading->set_visual_role(UiLabelVisualRole::Title);
    chrome->add_title_child(std::move(heading));
    auto body = std::make_unique<UiListContainer>(elysia::core::Rect{ 0,0,820,0 });
    body->set_padding(UiLayoutPadding{ 12,8,12,8 });
    body->set_item_spacing(8.0f);
    body->set_cross_align(UiLayoutAlign::Start);
    auto note = std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,760,28 },0,ui_text_key(description));
    note->set_visual_role(UiLabelVisualRole::Muted);
    body->add_back(std::move(note));
    UiListContainer* body_ptr = body.get();
    chrome->set_body(std::move(body));
    page.add_back(std::move(chrome));
    return body_ptr;
}

std::unique_ptr<UiLabel> status_label()
{
    auto label = std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,780,30 },0,ui_text_key("ui_test_scene.status.ready"));
    label->set_visual_role(UiLabelVisualRole::Subtitle);
    return label;
}
}

void UiContainerTestScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;
    _paused = false;
    rebuild_ui();
}

void UiContainerTestScene::on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events)
{
    ApplicationScene::on_input(input,events);
}

void UiContainerTestScene::on_exit() { _paused = false; clear_ui(); }
void UiContainerTestScene::reset() { _paused = false; clear_ui(); }

void UiContainerTestScene::refresh_theme_preview_styles()
{
    if (!_root_window)
        return;
    auto overrides = _root_window->style_overrides();
    overrides.draw_background = true;
    overrides.draw_border = true;
    _root_window->set_style_overrides(overrides);
}

void UiContainerTestScene::set_status(std::string text)
{
    if (_status_label)
        _status_label->set_text_content(elysia::ui::ui_raw_text(std::move(text)));
}

void UiContainerTestScene::set_active_theme(elysia::ui::UiBuiltinTheme theme)
{
    _theme_manager.set_theme(theme);
    refresh_theme_preview_styles();
    sync_theme_switch_button_roles();
    set_status("Theme changed");
}

void UiContainerTestScene::sync_theme_switch_button_roles() noexcept
{
    static constexpr std::array<elysia::ui::UiBuiltinTheme,7> themes{
        elysia::ui::UiBuiltinTheme::BlueGlassMoon, elysia::ui::UiBuiltinTheme::ElysiaLight,
        elysia::ui::UiBuiltinTheme::ElysiaDark, elysia::ui::UiBuiltinTheme::EvangelionUnit00,
        elysia::ui::UiBuiltinTheme::EvangelionUnit01, elysia::ui::UiBuiltinTheme::EvangelionUnit02,
        elysia::ui::UiBuiltinTheme::QuietSlate };
    for (std::size_t index = 0; index < themes.size(); ++index)
        if (_theme_buttons[index])
            _theme_buttons[index]->set_visual_role(_theme_manager.current_builtin_theme() == themes[index]
                ? elysia::ui::UiButtonVisualRole::Primary : elysia::ui::UiButtonVisualRole::Default);
}

void UiContainerTestScene::rebuild_ui()
{
    clear_ui();
    _root_window = Scene::create_and_add_object<UiWindow>(elysia::core::Rect{ 80,52,1120,616 },100);
    _root_window->set_padding(UiLayoutPadding{ 16,16,16,16 });
    _root_window->set_on_cancel([this]() { request_back_to_menu(); });
    _theme_registrations.push_back(_theme_manager.register_root(*_root_window));
    refresh_theme_preview_styles();

    auto status = status_label();
    _status_label = status.get();
    _root_window->add_child(std::move(status),at(16,12,850,30));

    auto workbench = std::make_unique<UiTabContainer>(elysia::core::Rect{ 0,0,1080,530 });
    UiTabContainer* tabs = workbench.get();

    // Overview: a concise health dashboard and representative themed roles.
    UiListContainer* overview_list = nullptr;
    auto overview = page_scroll(overview_list);
    auto* overview_section = add_section(*overview_list,"ui_test_scene.pages.overview","ui_test_scene.sections.overview");
    auto primary = button("ui_test_scene.actions.replay");
    primary->set_visual_role(UiButtonVisualRole::Primary);
    primary->set_on_click([this]() { set_status("Overview action invoked"); });
    overview_section->add_back(std::move(primary));
    auto danger = button("ui_test_scene.actions.reset");
    danger->set_visual_role(UiButtonVisualRole::Danger);
    danger->set_on_click([this]() { rebuild_ui(); });
    overview_section->add_back(std::move(danger));
    auto disabled = button("menu_scene.exit");
    disabled->set_enabled(false);
    overview_section->add_back(std::move(disabled));
    auto progress = std::make_unique<UiBar>(elysia::core::Rect{ 0,0,420,22 });
    progress->set_visual_role(UiBarVisualRole::Progress);
    progress->set_ratio(0.68f);
    overview_section->add_back(std::move(progress));
    (void)tabs->add_tab(ui_text_key("ui_test_scene.pages.overview"),std::move(overview));

    // Controls: each interactive control has an observable callback or state change.
    UiListContainer* controls_list = nullptr;
    auto controls = page_scroll(controls_list);
    auto* controls_section = add_section(*controls_list,"ui_test_scene.pages.controls","ui_test_scene.sections.controls");
    auto group = std::make_unique<UiButtonGroup>(elysia::core::Rect{ 0,0,320,120 });
    group->set_on_selection_changed([this](std::optional<std::size_t> index) { set_status(index ? "ButtonGroup selection changed" : "ButtonGroup cleared"); });
    group->add_button(button("menu_scene.start"));
    group->add_button(button("menu_scene.settings"));
    group->add_button(button("menu_scene.about"));
    controls_section->add_back(std::move(group));
    auto check = std::make_unique<UiCheckbox>(elysia::core::Rect{ 0,0,44,44 });
    check->set_mark_style(UiCheckboxMarkStyle::Checkmark);
    check->set_on_toggled([this](UiCheckboxState) { set_status("Checkbox toggled"); });
    controls_section->add_back(std::move(check));
    UiLabeledCheckboxConfig check_config{}; check_config.text_content = ui_text_key("ui_test_scene.controls.labeled_check");
    auto labeled_check = std::make_unique<UiLabeledCheckbox>(elysia::core::Rect{ 0,0,360,42 },check_config);
    controls_section->add_back(std::move(labeled_check));
    auto radios = std::make_unique<UiRadioGroup>(elysia::core::Rect{ 0,0,360,130 });
    radios->set_on_selection_changed([this](std::optional<std::size_t>) { set_status("Radio selection changed"); });
    radios->add_back(std::make_unique<UiRadioButton>(elysia::core::Rect{ 0,0,42,42 }));
    UiLabeledRadioButtonConfig radio_config{}; radio_config.text_content = ui_text_key("ui_test_scene.controls.labeled_radio");
    radios->add_back(std::make_unique<UiLabeledRadioButton>(elysia::core::Rect{ 0,0,300,42 },radio_config));
    controls_section->add_back(std::move(radios));
    auto slider = std::make_unique<UiSlider>(elysia::core::Rect{ 0,0,360,48 });
    slider->set_value_display(UiSliderValueDisplay::Percent); slider->set_value(0.5f);
    slider->set_on_value_changed([this](float) { set_status("Horizontal slider changed"); });
    controls_section->add_back(std::move(slider));
    auto vertical_slider = std::make_unique<UiSlider>(elysia::core::Rect{ 0,0,64,130 });
    vertical_slider->set_orientation(UiSliderOrientation::Vertical); vertical_slider->set_value_display(UiSliderValueDisplay::Value); vertical_slider->set_value(0.72f);
    controls_section->add_back(std::move(vertical_slider));
    auto input = std::make_unique<UiTextInput>(elysia::core::Rect{ 0,0,360,44 });
    input->set_placeholder_content(ui_text_key("ui_test_scene.controls.placeholder")); input->set_max_length(16);
    input->set_on_submit([this](std::string_view) { set_status("Text input submitted"); });
    controls_section->add_back(std::move(input));
    UiDragHandleConfig drag_config{}; drag_config.axis = UiDragAxis::Horizontal; drag_config.drag_bounds = elysia::core::Rect{ 0,0,400,50 };
    auto drag = std::make_unique<UiDragHandle>(elysia::core::Rect{ 0,0,28,28 },drag_config);
    drag->set_on_dragged([this](const elysia::core::Vector2&) { set_status("Drag handle moved"); });
    controls_section->add_back(std::move(drag));
    (void)tabs->add_tab(ui_text_key("ui_test_scene.pages.controls"),std::move(controls));

    // Content and media: safely hide resource-dependent samples when data is unavailable.
    UiListContainer* media_list = nullptr;
    auto media = page_scroll(media_list);
    auto* media_section = add_section(*media_list,"ui_test_scene.pages.media","ui_test_scene.sections.media");
    auto fade_label = std::make_unique<UiFadeLabel>(elysia::core::Rect{ 0,0,360,34 },0,ui_text_key("ui_test_scene.actions.replay"));
    fade_label->configure_playback(effects::UiOpacityFadeMode::FadeInOut,0.0,0.25,0.25); fade_label->play(); media_section->add_back(std::move(fade_label));
    auto blink_label = std::make_unique<UiBlinkLabel>(elysia::core::Rect{ 0,0,360,34 },0,ui_text_key("ui_test_scene.pages.media"));
    blink_label->configure_playback(effects::UiOpacityBlinkMode::VisibleFirst,0.0,0.2,0.2,2); blink_label->play(); media_section->add_back(std::move(blink_label));
    auto pulse_label = std::make_unique<UiPulseLabel>(elysia::core::Rect{ 0,0,360,34 },0,ui_text_key("ui_test_scene.sections.media"));
    pulse_label->configure_playback(effects::UiOpacityPulseMode::MinToMax,0.0,0.25,0.25,2); pulse_label->play(); media_section->add_back(std::move(pulse_label));
    auto text = std::make_unique<UiTextBlock>(elysia::core::Rect{ 0,0,700,84 });
    text->set_text_content(ui_text_key("ui_test_scene.media.long_text")); text->set_padding(6);
    media_section->add_back(std::move(text));
    auto number = std::make_unique<UiNumber>(elysia::core::Rect{ 0,0,240,42 });
    number->set_value(73.25); number->set_decimal_places(2); number->set_suffix(UiNumberSuffix::Percent);
    media_section->add_back(std::move(number));
    for (const auto direction : { BarFillDirection::LeftToRight,BarFillDirection::RightToLeft,BarFillDirection::TopToBottom,BarFillDirection::BottomToTop })
    {
        auto bar = std::make_unique<UiBar>(elysia::core::Rect{ 0,0,300,20 }); bar->set_ratio(0.58f); bar->set_fill_direction(direction);
        media_section->add_back(std::move(bar));
    }
    SDL_Texture* moon = elysia::resources::ResourceManager::instance()->find_texture("ui.moon");
    if (moon)
    {
        auto image = std::make_unique<UiImage>(moon,elysia::core::Rect{ 0,0,120,80 });
        media_section->add_back(std::move(image));
        auto fade = std::make_unique<UiFadeImage>(moon,elysia::core::Rect{ 0,0,120,80 });
        fade->configure_playback(effects::UiOpacityFadeMode::FadeInOut,0.1,0.35,0.35); fade->play(); media_section->add_back(std::move(fade));
        auto blink = std::make_unique<UiBlinkImage>(moon,elysia::core::Rect{ 0,0,120,80 });
        blink->configure_playback(effects::UiOpacityBlinkMode::VisibleFirst,0.0,0.25,0.25,2); blink->play(); media_section->add_back(std::move(blink));
        auto pulse = std::make_unique<UiPulseImage>(moon,elysia::core::Rect{ 0,0,120,80 });
        pulse->configure_playback(effects::UiOpacityPulseMode::MinToMax,0.0,0.3,0.3,2); pulse->play(); media_section->add_back(std::move(pulse));
    }
    else media_section->add_back(std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,500,32 },0,ui_text_key("ui_test_scene.media.texture_unavailable")));
    auto animation = std::make_unique<UiAnimation>("aozaki_aoko.idle",elysia::core::Rect{ 0,0,120,120 });
    const bool animation_loaded = animation->set_animation_key("aozaki_aoko.idle"); animation->set_visible(animation_loaded);
    if (animation_loaded) { animation->play(); media_section->add_back(std::move(animation)); }
    else media_section->add_back(std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,500,32 },0,ui_text_key("ui_test_scene.media.animation_unavailable")));
    (void)tabs->add_tab(ui_text_key("ui_test_scene.pages.media"),std::move(media));

    // Containers and layout.
    UiListContainer* containers_list = nullptr;
    auto containers = page_scroll(containers_list);
    auto* container_section = add_section(*containers_list,"ui_test_scene.pages.containers","ui_test_scene.sections.containers");
    auto grid = std::make_unique<UiGridContainer>(elysia::core::Rect{ 0,0,560,170 }); grid->set_column_count(3); grid->set_cell_spacing(elysia::core::Vector2(8,8));
    for (int index = 0; index < 6; ++index) grid->add_child(button(index % 2 ? "menu_scene.settings" : "menu_scene.start"));
    container_section->add_back(std::move(grid));
    auto chrome = std::make_unique<UiChromeContainer>(elysia::core::Rect{ 0,0,620,170 });
    chrome->add_left_action(button("common.back")); chrome->add_title_child(std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,240,30 },0,ui_text_key("ui_test_scene.containers.chrome")));
    chrome->add_right_action(button("common.close"));
    auto chrome_body = std::make_unique<UiPanel>(elysia::core::Rect{ 0,0,580,100 }); chrome_body->add_child(button("ui_test_scene.actions.replay")); chrome->set_body(std::move(chrome_body));
    container_section->add_back(std::move(chrome));
    auto nested_tabs = std::make_unique<UiTabContainer>(elysia::core::Rect{ 0,0,620,200 });
    auto nested_page = std::make_unique<UiPanel>(elysia::core::Rect{ 0,0,580,140 }); nested_page->add_child(button("menu_scene.about"));
    (void)nested_tabs->add_tab(ui_text_key("ui_test_scene.containers.nested_tab"),std::move(nested_page));
    container_section->add_back(std::move(nested_tabs));
    auto hidden_scroll = std::make_unique<UiScrollContainer>(elysia::core::Rect{ 0,0,500,110 });
    hidden_scroll->set_scroll_axis(UiScrollAxis::Horizontal); hidden_scroll->set_scrollbar_visibility(UiScrollBarVisibility::Hidden);
    auto hidden_content = std::make_unique<UiListContainer>(elysia::core::Rect{ 0,0,900,90 }); hidden_content->set_direction(UiListDirection::Horizontal);
    for (int index = 0; index < 5; ++index) hidden_content->add_back(button("menu_scene.about"));
    hidden_scroll->set_content(std::move(hidden_content)); container_section->add_back(std::move(hidden_scroll));
    (void)tabs->add_tab(ui_text_key("ui_test_scene.pages.containers"),std::move(containers));

    // Overlays, transient popups and focus recovery.
    UiListContainer* overlays_list = nullptr;
    auto overlays = page_scroll(overlays_list);
    auto* overlay_section = add_section(*overlays_list,"ui_test_scene.pages.overlays","ui_test_scene.sections.overlays");
    auto overlay = std::make_unique<UiPanel>(elysia::core::Rect{ 0,0,320,140 }); UiPanel* overlay_ptr = overlay.get();
    overlay->add_child(button("common.close")); _root_window->add_child(std::move(overlay),at(760,76,320,140));
    _root_window->register_overlay(*overlay_ptr,UiOverlayOptions{ .open=false,.modal=false,.close_on_cancel=true,.close_on_outside_click=true,.placement=UiOverlayPlacement::Center,.transition=UiOverlayTransition::Slide,.fallback_size=elysia::core::Vector2(320,140),.order=900 });
    auto open_overlay = button("ui_test_scene.overlays.open_overlay"); open_overlay->set_on_click([this,overlay_ptr]() { _root_window->open_overlay(*overlay_ptr); set_status("Non-modal overlay opened"); }); overlay_section->add_back(std::move(open_overlay));
    auto dialog = std::make_unique<UiDialog>(elysia::core::Rect{ 0,0,480,300 }); UiDialog* dialog_ptr = dialog.get();
    dialog->set_title_content(ui_text_key("ui_test_scene.dialog.title")); dialog->set_body_content(ui_text_key("ui_test_scene.dialog.body")); dialog->set_action_content(ui_text_key("common.close"));
    _root_window->add_child(std::move(dialog),at(760,230,480,300)); dialog_ptr->register_with_window(*_root_window);
    auto open_dialog = button("ui_test_scene.overlays.open_dialog"); open_dialog->set_on_click([dialog_ptr]() { dialog_ptr->open(); }); overlay_section->add_back(std::move(open_dialog));
    auto confirm = std::make_unique<UiConfirmationDialog>(elysia::core::Rect{ 0,0,440,220 }); UiConfirmationDialog* confirm_ptr = confirm.get();
    confirm->set_config(UiConfirmationDialogConfig{ .title=ui_text_key("ui_test_scene.confirm.title"),.message=ui_text_key("ui_test_scene.confirm.message"),.confirm=ui_text_key("common.confirm"),.cancel=ui_text_key("common.cancel"),.close=ui_text_key("common.close") });
    _root_window->add_child(std::move(confirm),at(760,230,440,220)); confirm_ptr->register_with_window(*_root_window);
    auto open_confirm = button("ui_test_scene.overlays.open_confirm"); open_confirm->set_on_click([confirm_ptr]() { confirm_ptr->open(); }); overlay_section->add_back(std::move(open_confirm));
    auto dropdown = std::make_unique<UiDropdown>(elysia::core::Rect{ 0,0,320,42 });
    dropdown->set_options({ UiDropdownOption{ui_text_key("menu_scene.start")},UiDropdownOption{ui_text_key("menu_scene.settings")},UiDropdownOption{ui_text_key("menu_scene.about")},UiDropdownOption{ui_text_key("menu_scene.exit")} });
    dropdown->register_with_window(*_root_window); overlay_section->add_back(std::move(dropdown));
    auto tooltip_trigger = button("ui_test_scene.overlays.tooltip");
    UiButton* tooltip_trigger_ptr = tooltip_trigger.get();
    overlay_section->add_back(std::move(tooltip_trigger));
    auto* tooltip = _root_window->create_child<UiTooltip>(0);
    auto tooltip_content = std::make_unique<UiTextBlock>(elysia::core::Rect{ 0,0,280,72 });
    tooltip_content->set_text_content(ui_text_key("ui_test_scene.overlays.tooltip_text")); tooltip_content->set_padding(8);
    tooltip->bind_trigger(*tooltip_trigger_ptr); tooltip->set_content(std::move(tooltip_content)); tooltip->register_with_window(*_root_window);
    (void)tabs->add_tab(ui_text_key("ui_test_scene.pages.overlays"),std::move(overlays));

    // Theme and localization page.
    UiListContainer* theme_list = nullptr;
    auto themes_page = page_scroll(theme_list);
    auto* theme_section = add_section(*theme_list,"ui_test_scene.pages.theme","ui_test_scene.sections.theme");
    static constexpr std::array<const char*,7> theme_keys{ "ui_test_scene.theme_blue_glass_moon","ui_test_scene.theme_elysia_light","ui_test_scene.theme_elysia_dark","ui_test_scene.theme_evangelion_unit_00","ui_test_scene.theme_evangelion_unit_01","ui_test_scene.theme_evangelion_unit_02","ui_test_scene.theme_quiet_slate" };
    static constexpr std::array<UiBuiltinTheme,7> themes{ UiBuiltinTheme::BlueGlassMoon,UiBuiltinTheme::ElysiaLight,UiBuiltinTheme::ElysiaDark,UiBuiltinTheme::EvangelionUnit00,UiBuiltinTheme::EvangelionUnit01,UiBuiltinTheme::EvangelionUnit02,UiBuiltinTheme::QuietSlate };
    for (std::size_t index = 0; index < themes.size(); ++index) { auto theme_button = button(theme_keys[index]); _theme_buttons[index] = theme_button.get(); theme_button->set_on_click([this,theme=themes[index]]() { set_active_theme(theme); }); theme_section->add_back(std::move(theme_button)); }
    auto raw = std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,680,32 },0,ui_raw_text("Raw text: localization bypass comparison")); raw->set_visual_role(UiLabelVisualRole::Muted); theme_section->add_back(std::move(raw));
    (void)tabs->add_tab(ui_text_key("ui_test_scene.pages.theme"),std::move(themes_page));

    _root_window->add_child(std::move(workbench),at(16,48,1080,530));
    _root_window->register_focus_scope(*tabs);
    _root_window->focus_first_available_scope();
    sync_theme_switch_button_roles();
}

void UiContainerTestScene::clear_ui()
{
    _theme_registrations.clear();
    _theme_buttons.fill(nullptr);
    _status_label = nullptr;
    if (_root_window) { _root_window->destroy(); _root_window = nullptr; }
}

void UiContainerTestScene::request_back_to_menu()
{
    request_scene_switch(AppSceneKeys::MainMenu);
}
}
