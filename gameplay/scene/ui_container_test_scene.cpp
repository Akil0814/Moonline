#include "ui_container_test_scene.h"


#include "../../application/scene/scene_keys.h"


#include "../../engine/ui/window/ui_window.h"
#include "../../engine/ui/containers/ui_grid_container.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_radio_group.h"
#include "../../engine/ui/containers/ui_scroll_container.h"
#include "../../engine/ui/containers/ui_dialog.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/ui_dropdown_button_set.h"
#include "../../engine/ui/widgets/ui_checkbox.h"
#include "../../engine/ui/widgets/ui_labeled_checkbox.h"
#include "../../engine/ui/widgets/ui_radio_button.h"
#include "../../engine/ui/widgets/ui_slider.h"
#include "../../engine/ui/widgets/ui_text_input.h"
#include "../../engine/ui/widgets/ui_tooltip.h"
#include "../../engine/ui/widgets/text/ui_text_block.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/layout/ui_layout_types.h"

#include <array>
#include <iostream>
#include <memory>
#include <utility>

namespace arcneco::scene
{
namespace
{
template<class Widget,class Mutator>
void update_style(Widget& widget,Mutator&& mutator)
{
    auto style = widget.style();
    mutator(style);
    widget.set_style(style);
}

[[nodiscard]] elysia::ui::UiButtonConfig make_button_config(const char* text_key)
{
    return elysia::ui::UiButtonConfig{ .content = elysia::ui::ui_text_key(text_key) };
}

[[nodiscard]] const char* button_text_key_for_index(int index) noexcept
{
    static constexpr std::array<const char*,4> kButtonTextKeys{
        "menu_scene.start",
        "menu_scene.settings",
        "menu_scene.about",
        "menu_scene.exit"
    };
    return kButtonTextKeys[static_cast<std::size_t>(index) % kButtonTextKeys.size()];
}

[[nodiscard]] elysia::ui::UiLayoutChildOptions make_window_child_options(float left,float top)
{
    return elysia::ui::UiLayoutChildOptions{
        ._anchor = elysia::ui::UiLayoutAnchor::TopLeft,
        ._margin = elysia::ui::UiLayoutMargin{ left,top,0.0f,0.0f },
        ._cross_align = elysia::ui::UiLayoutAlign::Start,
        ._size_override = elysia::core::Vector2(0.0f,0.0f),
        ._use_custom_cross_align = false,
        ._fill_cross_axis = false,
        ._use_size_override = false
    };
}

[[nodiscard]] elysia::ui::UiLayoutChildOptions make_theme_button_options(float left,float top)
{
    return elysia::ui::UiLayoutChildOptions{
        ._anchor = elysia::ui::UiLayoutAnchor::TopLeft,
        ._margin = elysia::ui::UiLayoutMargin{ left,top,0.0f,0.0f },
        ._cross_align = elysia::ui::UiLayoutAlign::Start,
        ._size_override = elysia::core::Vector2(140.0f,36.0f),
        ._use_custom_cross_align = false,
        ._fill_cross_axis = false,
        ._use_size_override = true
    };
}

std::unique_ptr<elysia::ui::UiButton> make_button(int index,const char* scope)
{
    auto button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 0,0,180,44 },
        make_button_config(button_text_key_for_index(index)),
        0);
    button->set_on_click([index,scope]()
    {
        std::cout << scope << " button " << index << std::endl;
    });
    return button;
}

std::unique_ptr<elysia::ui::UiCheckbox> make_checkbox(
    const elysia::core::Rect& rect,
    bool checked,
    elysia::ui::UiCheckboxMarkStyle mark_style,
    const char* scope
)
{
    auto checkbox = std::make_unique<elysia::ui::UiCheckbox>(rect,0);
    checkbox->set_checked(checked);
    checkbox->set_mark_style(mark_style);
    checkbox->set_on_toggled([scope](elysia::ui::UiCheckboxState state)
    {
        std::cout << scope << " checkbox state " << static_cast<int>(state) << std::endl;
    });
    return checkbox;
}

std::unique_ptr<elysia::ui::UiLabeledCheckbox> make_labeled_checkbox(
    const elysia::core::Rect& rect,
    const char* text_key,
    bool checked,
    elysia::ui::UiLabeledCheckboxLabelPlacement label_placement,
    elysia::ui::UiLabeledCheckboxTextPlacement text_placement,
    bool draw_background,
    bool draw_border,
    const char* scope
)
{
    elysia::ui::UiLabeledCheckboxConfig config{};
    config.text_content = elysia::ui::ui_text_key(text_key);
    config.label_placement = label_placement;
    config.text_placement = text_placement;
    config.draw_background = draw_background;
    config.draw_border = draw_border;

    auto checkbox = std::make_unique<elysia::ui::UiLabeledCheckbox>(rect,config,0);
    checkbox->set_checked(checked);
    checkbox->set_on_toggled([scope](elysia::ui::UiCheckboxState state)
    {
        std::cout << scope << " labeled checkbox state " << static_cast<int>(state) << std::endl;
    });
    return checkbox;
}

std::unique_ptr<elysia::ui::UiTextInput> make_text_input(
    const elysia::core::Rect& rect,
    std::string placeholder_text,
    std::optional<std::size_t> max_length,
    const char* scope
)
{
    auto input = std::make_unique<elysia::ui::UiTextInput>(rect,0);
    input->set_placeholder_content(elysia::ui::ui_raw_text(std::move(placeholder_text)));
    input->set_max_length(max_length);
    input->set_on_text_changed([scope](std::string_view text)
    {
        std::cout << scope << " text changed: " << text << std::endl;
    });
    input->set_on_submit([scope](std::string_view text)
    {
        std::cout << scope << " submit: " << text << std::endl;
    });
    return input;
}

std::unique_ptr<elysia::ui::UiSlider> make_slider(
    const elysia::core::Rect& rect,
    const char* text_key,
    float value,
    const char* scope
)
{
    auto slider = std::make_unique<elysia::ui::UiSlider>(rect,0);
    slider->set_text_content(elysia::ui::ui_text_key(text_key));
    slider->set_label_placement(elysia::ui::UiSliderLabelPlacement::Above);
    slider->set_value_label_mode(elysia::ui::UiSliderValueLabelMode::Percent);
    slider->set_range(0.0f,1.0f);
    slider->set_value(value);
    slider->set_on_value_changed([scope](float changed_value)
    {
        std::cout << scope << " slider value " << changed_value << std::endl;
    });
    return slider;
}

std::unique_ptr<elysia::ui::UiRadioButton> make_radio_button(
    const elysia::core::Rect& rect,
    const char* text_key,
    bool selected,
    const char* scope
)
{
    auto radio_button = std::make_unique<elysia::ui::UiRadioButton>(rect,0);
    radio_button->set_text_content(elysia::ui::ui_text_key(text_key));
    radio_button->set_selected(selected);
    radio_button->set_on_selected([scope]()
    {
        std::cout << scope << " radio selected" << std::endl;
    });
    return radio_button;
}
}

void UiContainerTestScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;
    _paused = false;
    rebuild_ui();
}

void UiContainerTestScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    ApplicationScene::on_input(input,events);
}

void UiContainerTestScene::on_exit()
{
    _paused = false;
    clear_ui();
}

void UiContainerTestScene::reset()
{
    _paused = false;
    clear_ui();
}

void UiContainerTestScene::register_theme_element(elysia::ui::UiElement& element)
{
    element.set_use_theme(true);
    _theme_registrations.push_back(_theme_manager.register_element(element));
}

void UiContainerTestScene::refresh_theme_preview_styles()
{
    if (!_root_window)
        return;

    elysia::ui::UiWindowStyle window_style = elysia::ui::apply_theme_colors(
        elysia::ui::UiWindowStyle{},
        _theme_manager.current_theme().window_style);
    window_style.draw_background = true;
    window_style.draw_border = true;
    _root_window->set_style(window_style);
}

void UiContainerTestScene::set_active_theme(elysia::ui::UiBuiltinTheme theme)
{
    _theme_manager.set_theme(theme);
    refresh_theme_preview_styles();
    sync_theme_switch_button_roles();

    switch (theme)
    {
    case elysia::ui::UiBuiltinTheme::BlueGlassMoon:
        std::cout << "ui test theme switched: BlueGlassMoon" << std::endl;
        break;
    case elysia::ui::UiBuiltinTheme::ElysiaLight:
        std::cout << "ui test theme switched: ElysiaLight" << std::endl;
        break;
    case elysia::ui::UiBuiltinTheme::ElysiaDark:
        std::cout << "ui test theme switched: ElysiaDark" << std::endl;
        break;
    case elysia::ui::UiBuiltinTheme::EvangelionUnit00:
        std::cout << "ui test theme switched: EvangelionUnit00" << std::endl;
        break;
    case elysia::ui::UiBuiltinTheme::EvangelionUnit01:
        std::cout << "ui test theme switched: EvangelionUnit01" << std::endl;
        break;
    case elysia::ui::UiBuiltinTheme::EvangelionUnit02:
        std::cout << "ui test theme switched: EvangelionUnit02" << std::endl;
        break;
    case elysia::ui::UiBuiltinTheme::QuietSlate:
        std::cout << "ui test theme switched: QuietSlate" << std::endl;
        break;
    }
}

void UiContainerTestScene::sync_theme_switch_button_roles() noexcept
{
    const elysia::ui::UiBuiltinTheme current_theme = _theme_manager.current_builtin_theme();
    const auto sync_role = [current_theme](elysia::ui::UiButton* button,elysia::ui::UiBuiltinTheme theme)
    {
        if (!button)
            return;

        button->set_theme_role(
            current_theme == theme
            ? elysia::ui::UiButtonThemeRole::Primary
            : elysia::ui::UiButtonThemeRole::Default);
    };

    sync_role(_blue_glass_moon_theme_button,elysia::ui::UiBuiltinTheme::BlueGlassMoon);
    sync_role(_elysia_light_theme_button,elysia::ui::UiBuiltinTheme::ElysiaLight);
    sync_role(_elysia_dark_theme_button,elysia::ui::UiBuiltinTheme::ElysiaDark);
    sync_role(_evangelion_unit_00_theme_button,elysia::ui::UiBuiltinTheme::EvangelionUnit00);
    sync_role(_evangelion_unit_01_theme_button,elysia::ui::UiBuiltinTheme::EvangelionUnit01);
    sync_role(_evangelion_unit_02_theme_button,elysia::ui::UiBuiltinTheme::EvangelionUnit02);
    sync_role(_quiet_slate_theme_button,elysia::ui::UiBuiltinTheme::QuietSlate);
}

void UiContainerTestScene::rebuild_ui()
{
    clear_ui();

    const auto register_themed = [this](elysia::ui::UiElement* element)
    {
        if (element)
            register_theme_element(*element);
    };

    _root_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 120,80,1040,560 },100);
    _root_window->set_padding(elysia::ui::UiLayoutPadding{ 24.0f,24.0f,24.0f,24.0f });
    _root_window->set_on_cancel([this]()
    {
        request_back_to_menu();
    });
    register_themed(_root_window);
    refresh_theme_preview_styles();

    auto* vertical_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,260,420 });
    vertical_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    vertical_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    vertical_scroll->set_scroll_step(elysia::core::Vector2(32.0f,32.0f));
    register_themed(vertical_scroll);

    auto vertical_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,260,760 });
    vertical_list->set_padding(elysia::ui::UiLayoutPadding{ 20.0f,20.0f,20.0f,20.0f });
    vertical_list->set_item_spacing(18.0f);
    for (int index = 0; index < 4; ++index)
        register_themed(vertical_list->add_back(make_button(index,"vertical")));

    auto nested_radio_group = std::make_unique<elysia::ui::UiRadioGroup>(elysia::core::Rect{ 0,0,220,54 });
    nested_radio_group->set_direction(elysia::ui::UiListDirection::Horizontal);
    nested_radio_group->set_item_spacing(14.0f);
    nested_radio_group->set_padding(elysia::ui::UiLayoutPadding{ 10.0f,8.0f,10.0f,8.0f });
    nested_radio_group->set_on_selection_changed([](std::optional<std::size_t> selected_index)
    {
        if (selected_index)
            std::cout << "vertical nested radio group " << *selected_index << std::endl;
        else
            std::cout << "vertical nested radio group none" << std::endl;
    });
    register_themed(nested_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,60,36 },
        "menu_scene.start",
        true,
        "vertical-nested")));
    register_themed(nested_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,60,36 },
        "menu_scene.settings",
        false,
        "vertical-nested")));
    register_themed(nested_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,60,36 },
        "menu_scene.about",
        false,
        "vertical-nested")));
    vertical_list->add_back(std::move(nested_radio_group));

    for (int index = 4; index < 10; ++index)
        register_themed(vertical_list->add_back(make_button(index,"vertical")));
    vertical_scroll->set_content(std::move(vertical_list));

    auto* horizontal_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,180 });
    horizontal_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    horizontal_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    horizontal_scroll->set_scroll_step(elysia::core::Vector2(36.0f,36.0f));
    register_themed(horizontal_scroll);

    auto horizontal_panel = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,760,180 });
    horizontal_panel->set_theme_role(elysia::ui::UiPanelThemeRole::Dialog);

    auto horizontal_button_0 = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 70,68,120,44 },
        make_button_config(button_text_key_for_index(0)),
        0);
    horizontal_button_0->set_on_click([]()
    {
        std::cout << "horizontal button 0" << std::endl;
    });
    register_themed(horizontal_button_0.get());
    horizontal_panel->add_child(std::move(horizontal_button_0),elysia::ui::UiPanelInsertDirection::Down);

    for (int index = 1; index < 5; ++index)
    {
        auto button = std::make_unique<elysia::ui::UiButton>(
            elysia::core::Rect{ 70.0f + 140.0f * static_cast<float>(index),68.0f,120.0f,44.0f },
            make_button_config(button_text_key_for_index(index)),
            0);
        button->set_on_click([index]()
        {
            std::cout << "horizontal button " << index << std::endl;
        });
        register_themed(button.get());
        horizontal_panel->add_child(std::move(button),elysia::ui::UiPanelInsertDirection::Right);
    }
    register_themed(horizontal_panel.get());
    horizontal_scroll->set_content(std::move(horizontal_panel));

    auto* grid_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,340,260 });
    grid_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    grid_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    grid_scroll->set_scroll_step(elysia::core::Vector2(28.0f,28.0f));
    register_themed(grid_scroll);

    auto grid_content = std::make_unique<elysia::ui::UiGridContainer>(elysia::core::Rect{ 0,0,520,360 });
    grid_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    grid_content->set_column_count(4);
    grid_content->set_cell_spacing(elysia::core::Vector2(16.0f,16.0f));
    for (int index = 0; index < 20; ++index)
    {
        auto button = std::make_unique<elysia::ui::UiButton>(
            elysia::core::Rect{ 0,0,96,48 },
            make_button_config(button_text_key_for_index(index)),
            0);
        button->set_on_click([index]()
        {
            std::cout << "grid button " << index << std::endl;
        });
        register_themed(button.get());
        grid_content->add_child(std::move(button));
    }
    grid_scroll->set_content(std::move(grid_content));

    auto* hidden_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,180 });
    hidden_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    hidden_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Hidden);
    hidden_scroll->set_scroll_step(elysia::core::Vector2(30.0f,30.0f));
    register_themed(hidden_scroll);

    auto first_hidden_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,320,360 });
    first_hidden_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    first_hidden_content->set_item_spacing(14.0f);
    for (int index = 0; index < 4; ++index)
        register_themed(first_hidden_content->add_back(make_button(index,"hidden-initial")));
    hidden_scroll->set_content(std::move(first_hidden_content));

    auto replacement_hidden_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,320,520 });
    replacement_hidden_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    replacement_hidden_content->set_item_spacing(14.0f);
    for (int index = 0; index < 8; ++index)
        register_themed(replacement_hidden_content->add_back(make_button(index,"hidden")));
    hidden_scroll->set_content(std::move(replacement_hidden_content));

    auto* widget_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,340,220 });
    widget_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    widget_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    widget_scroll->set_scroll_step(elysia::core::Vector2(24.0f,24.0f));
    register_themed(widget_scroll);

    auto widget_content = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,340,1160 });
    widget_content->set_theme_role(elysia::ui::UiPanelThemeRole::Dialog);
    register_themed(widget_content.get());

    auto* non_modal_overlay = _root_window->create_child<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,260,128 });
    non_modal_overlay->set_theme_role(elysia::ui::UiPanelThemeRole::Dialog);
    register_themed(non_modal_overlay);
    auto overlay_close_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 40,42,180,44 },
        make_button_config("menu_scene.exit"),
        0);
    overlay_close_button->set_on_click([this,non_modal_overlay]()
    {
        if (_root_window)
            _root_window->set_overlay_open(*non_modal_overlay,false);
    });
    register_themed(overlay_close_button.get());
    non_modal_overlay->add_child(std::move(overlay_close_button),elysia::ui::UiPanelInsertDirection::Down);
    _root_window->register_overlay(
        *non_modal_overlay,
        elysia::ui::UiOverlayOptions{
            .open = false,
            .modal = false,
            .close_on_cancel = true,
            .close_on_outside_click = true,
            .placement = elysia::ui::UiOverlayPlacement::Center,
            .transition = elysia::ui::UiOverlayTransition::None,
            .fallback_size = elysia::core::Vector2(260.0f,128.0f),
            .order = 900
        });

    auto* localized_dialog = _root_window->create_child<elysia::ui::UiDialog>(elysia::core::Rect{ 0,0,520,360 });
    localized_dialog->set_title_content(elysia::ui::ui_text_key("ui_test_scene.reading_dialog.title"));
    localized_dialog->set_body_content(elysia::ui::ui_text_key("ui_test_scene.reading_dialog.body"));
    localized_dialog->set_action_content(elysia::ui::ui_text_key("ui_test_scene.reading_dialog.close"));
    localized_dialog->register_as_overlay(*_root_window);
    register_themed(localized_dialog);

    auto* raw_text_dialog = _root_window->create_child<elysia::ui::UiDialog>(elysia::core::Rect{ 0,0,500,340 });
    raw_text_dialog->set_title_content(elysia::ui::ui_raw_text("Raw Text Preview"));
    raw_text_dialog->set_body_content(elysia::ui::ui_raw_text(
        "This dialog bypasses the localization dictionary on purpose.\n\n"
        "It is useful for debug notes, generated copy, or temporary editor strings.\n\n"
        "The body text still wraps and scrolls like the i18n-backed reading dialog."));
    raw_text_dialog->set_action_content(elysia::ui::ui_raw_text("Close"));
    raw_text_dialog->register_as_overlay(*_root_window);
    register_themed(raw_text_dialog);

    auto checkbox_0 = make_checkbox(
        elysia::core::Rect{ 18,18,36,36 },
        false,
        elysia::ui::UiCheckboxMarkStyle::Checkmark,
        "widget");
    register_themed(checkbox_0.get());
    widget_content->add_child(std::move(checkbox_0),elysia::ui::UiPanelInsertDirection::Down);

    auto checkbox_1 = make_checkbox(
        elysia::core::Rect{ 74,18,36,36 },
        true,
        elysia::ui::UiCheckboxMarkStyle::FilledBox,
        "widget");
    register_themed(checkbox_1.get());
    widget_content->add_child(std::move(checkbox_1),elysia::ui::UiPanelInsertDirection::Right);

    auto labeled_checkbox_0 = make_labeled_checkbox(
        elysia::core::Rect{ 18,74,220,40 },
        "menu_scene.settings",
        false,
        elysia::ui::UiLabeledCheckboxLabelPlacement::Right,
        elysia::ui::UiLabeledCheckboxTextPlacement::NearBox,
        false,
        false,
        "widget");
    register_themed(labeled_checkbox_0.get());
    widget_content->add_child(std::move(labeled_checkbox_0),elysia::ui::UiPanelInsertDirection::Down);

    auto labeled_checkbox_1 = make_labeled_checkbox(
        elysia::core::Rect{ 18,124,220,40 },
        "menu_scene.about",
        true,
        elysia::ui::UiLabeledCheckboxLabelPlacement::Right,
        elysia::ui::UiLabeledCheckboxTextPlacement::FarEdge,
        true,
        true,
        "widget");
    register_themed(labeled_checkbox_1.get());
    widget_content->add_child(std::move(labeled_checkbox_1),elysia::ui::UiPanelInsertDirection::Down);

    auto labeled_checkbox_2 = make_labeled_checkbox(
        elysia::core::Rect{ 18,174,220,40 },
        "menu_scene.start",
        false,
        elysia::ui::UiLabeledCheckboxLabelPlacement::Left,
        elysia::ui::UiLabeledCheckboxTextPlacement::NearBox,
        false,
        false,
        "widget");
    register_themed(labeled_checkbox_2.get());
    widget_content->add_child(std::move(labeled_checkbox_2),elysia::ui::UiPanelInsertDirection::Down);

    auto labeled_checkbox_3 = make_labeled_checkbox(
        elysia::core::Rect{ 18,224,220,40 },
        "menu_scene.exit",
        true,
        elysia::ui::UiLabeledCheckboxLabelPlacement::Left,
        elysia::ui::UiLabeledCheckboxTextPlacement::FarEdge,
        true,
        true,
        "widget");
    register_themed(labeled_checkbox_3.get());
    widget_content->add_child(std::move(labeled_checkbox_3),elysia::ui::UiPanelInsertDirection::Down);

    auto vertical_radio_group = std::make_unique<elysia::ui::UiRadioGroup>(elysia::core::Rect{ 18,282,280,154 });
    vertical_radio_group->set_padding(elysia::ui::UiLayoutPadding{ 10.0f,10.0f,10.0f,10.0f });
    vertical_radio_group->set_item_spacing(10.0f);
    vertical_radio_group->set_on_selection_changed([](std::optional<std::size_t> selected_index)
    {
        if (selected_index)
            std::cout << "widget vertical radio group " << *selected_index << std::endl;
        else
            std::cout << "widget vertical radio group none" << std::endl;
    });
    register_themed(vertical_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,240,38 },
        "menu_scene.start",
        true,
        "widget-vertical")));
    register_themed(vertical_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,240,38 },
        "menu_scene.settings",
        true,
        "widget-vertical")));
    register_themed(vertical_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,240,38 },
        "menu_scene.about",
        false,
        "widget-vertical")));
    widget_content->add_child(std::move(vertical_radio_group),elysia::ui::UiPanelInsertDirection::Down);

    auto horizontal_radio_group = std::make_unique<elysia::ui::UiRadioGroup>(elysia::core::Rect{ 18,452,280,54 });
    horizontal_radio_group->set_direction(elysia::ui::UiListDirection::Horizontal);
    horizontal_radio_group->set_item_spacing(16.0f);
    horizontal_radio_group->set_padding(elysia::ui::UiLayoutPadding{ 10.0f,8.0f,10.0f,8.0f });
    horizontal_radio_group->set_on_selection_changed([](std::optional<std::size_t> selected_index)
    {
        if (selected_index)
            std::cout << "widget horizontal radio group " << *selected_index << std::endl;
        else
            std::cout << "widget horizontal radio group none" << std::endl;
    });
    register_themed(horizontal_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,84,36 },
        "menu_scene.start",
        false,
        "widget-horizontal")));
    register_themed(horizontal_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,84,36 },
        "menu_scene.settings",
        false,
        "widget-horizontal")));
    register_themed(horizontal_radio_group->add_back(make_radio_button(
        elysia::core::Rect{ 0,0,84,36 },
        "menu_scene.about",
        false,
        "widget-horizontal")));
    widget_content->add_child(std::move(horizontal_radio_group),elysia::ui::UiPanelInsertDirection::Down);

    auto text_input_main = make_text_input(
        elysia::core::Rect{ 18,526,280,44 },
        "Type here",
        std::nullopt,
        "widget-main");
    register_themed(text_input_main.get());
    widget_content->add_child(std::move(text_input_main),elysia::ui::UiPanelInsertDirection::Down);

    auto text_input_limited = make_text_input(
        elysia::core::Rect{ 18,580,280,44 },
        "Max 8 chars",
        std::optional<std::size_t>(8),
        "widget-limited");
    register_themed(text_input_limited.get());
    widget_content->add_child(std::move(text_input_limited),elysia::ui::UiPanelInsertDirection::Down);

    auto slider = make_slider(
        elysia::core::Rect{ 18,638,280,78 },
        "menu_scene.start",
        0.42f,
        "widget");
    register_themed(slider.get());
    widget_content->add_child(std::move(slider),elysia::ui::UiPanelInsertDirection::Down);

    auto open_overlay_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 18,732,150,40 },
        make_button_config("menu_scene.about"),
        0);
    open_overlay_button->set_on_click([this,non_modal_overlay]()
    {
        if (_root_window)
            _root_window->set_overlay_open(*non_modal_overlay,true);
    });
    register_themed(open_overlay_button.get());
    widget_content->add_child(std::move(open_overlay_button),elysia::ui::UiPanelInsertDirection::Down);

    auto open_localized_dialog_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 18,780,150,40 },
        make_button_config("ui_test_scene.reading_dialog.open"),
        0);
    open_localized_dialog_button->set_on_click([this,localized_dialog]()
    {
        if (_root_window && localized_dialog)
            localized_dialog->open(*_root_window);
    });
    register_themed(open_localized_dialog_button.get());
    widget_content->add_child(std::move(open_localized_dialog_button),elysia::ui::UiPanelInsertDirection::Down);

    auto open_raw_dialog_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 18,830,150,40 },
        make_button_config("ui_test_scene.reading_dialog.open_raw"),
        0);
    open_raw_dialog_button->set_on_click([this,raw_text_dialog]()
    {
        if (_root_window && raw_text_dialog)
            raw_text_dialog->open(*_root_window);
    });
    register_themed(open_raw_dialog_button.get());
    widget_content->add_child(std::move(open_raw_dialog_button),elysia::ui::UiPanelInsertDirection::Down);

    auto dropdown = std::make_unique<elysia::ui::UiDropdownButtonSet>(elysia::core::Rect{ 18,882,280,42 });
    dropdown->set_options({
        elysia::ui::UiDropdownOption{ elysia::ui::ui_text_key("menu_scene.start") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_text_key("menu_scene.settings") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_text_key("menu_scene.about") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_text_key("menu_scene.exit") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_raw_text("Raw option A") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_raw_text("Raw option B") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_raw_text("Raw option C") },
        elysia::ui::UiDropdownOption{ elysia::ui::ui_raw_text("Raw option D") }
    });
    update_style(*dropdown,[](elysia::ui::UiDropdownButtonSetStyle& style)
    {
        style.popup_max_height = 126.0f;
    });
    dropdown->set_on_selection_changed([](std::size_t selected_index)
    {
        std::cout << "widget dropdown selected " << selected_index << std::endl;
    });
    dropdown->register_as_transient_popup(*_root_window);
    register_themed(dropdown.get());
    widget_content->add_child(std::move(dropdown),elysia::ui::UiPanelInsertDirection::Down);

    auto text_tooltip_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 18,934,220,40 },
        make_button_config("menu_scene.about"),
        0);
    elysia::ui::UiButton* text_tooltip_trigger = text_tooltip_button.get();
    register_themed(text_tooltip_button.get());
    widget_content->add_child(std::move(text_tooltip_button),elysia::ui::UiPanelInsertDirection::Down);

    auto panel_tooltip_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 18,982,220,40 },
        make_button_config("menu_scene.settings"),
        0);
    elysia::ui::UiButton* panel_tooltip_trigger = panel_tooltip_button.get();
    register_themed(panel_tooltip_button.get());
    widget_content->add_child(std::move(panel_tooltip_button),elysia::ui::UiPanelInsertDirection::Down);

    auto* text_tooltip = _root_window->create_child<elysia::ui::UiTooltip>(0);
    auto text_tooltip_content = std::make_unique<elysia::ui::UiTextBlock>(elysia::core::Rect{ 0,0,260,82 });
    text_tooltip_content->set_text_content(elysia::ui::ui_raw_text(
        "Tooltip: hover this button or focus it with keyboard/gamepad."));
    text_tooltip_content->set_padding(10);
    register_themed(text_tooltip_content.get());
    text_tooltip->bind_trigger(*text_tooltip_trigger);
    text_tooltip->set_content(std::move(text_tooltip_content));
    text_tooltip->register_with_window(*_root_window);

    auto* panel_tooltip = _root_window->create_child<elysia::ui::UiTooltip>(0);
    auto panel_tooltip_content = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,250,104 });
    panel_tooltip_content->set_padding(elysia::ui::UiLayoutPadding{ 10.0f,10.0f,10.0f,10.0f });
    panel_tooltip_content->set_theme_role(elysia::ui::UiPanelThemeRole::Dialog);
    auto panel_tooltip_label = std::make_unique<elysia::ui::UiLabel>(
        elysia::core::Rect{ 0,0,220,36 },0,elysia::ui::ui_raw_text("Moved composite UiPanel"));
    register_themed(panel_tooltip_label.get());
    panel_tooltip_content->add_child(std::move(panel_tooltip_label),elysia::ui::UiPanelInsertDirection::Down);
    register_themed(panel_tooltip_content.get());
    panel_tooltip->bind_trigger(*panel_tooltip_trigger);
    panel_tooltip->set_content(std::move(panel_tooltip_content));
    panel_tooltip->register_with_window(*_root_window);

    auto anchored_options_button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 0,0,132,36 },
        make_button_config("menu_scene.settings"),
        0);
    anchored_options_button->set_on_click([]()
    {
        std::cout << "widget anchored options button" << std::endl;
    });
    register_themed(anchored_options_button.get());
    widget_content->add_child(
        std::move(anchored_options_button),
        elysia::ui::UiLayoutChildOptions{
            ._anchor = elysia::ui::UiLayoutAnchor::BottomRight,
            ._margin = elysia::ui::UiLayoutMargin{ 0.0f,0.0f,18.0f,18.0f },
            ._cross_align = elysia::ui::UiLayoutAlign::Start,
            ._size_override = elysia::core::Vector2(132.0f,36.0f),
            ._use_custom_cross_align = false,
            ._fill_cross_axis = false,
            ._use_size_override = true
        });
    widget_scroll->set_content(std::move(widget_content));

    const auto configure_theme_button =
        [this,&register_themed](
            elysia::ui::UiButton*& button_slot,
            float left,
            float top,
            const char* text_key,
            elysia::ui::UiBuiltinTheme theme)
    {
        button_slot = _root_window->create_child<elysia::ui::UiButton>(
            make_theme_button_options(left,top),
            elysia::core::Rect{ 0,0,140,36 },
            make_button_config(text_key),
            0);
        if (!button_slot)
            return;

        button_slot->set_typography_role(elysia::ui::UiTypographyRole::ButtonCompact);
        button_slot->set_on_click([this,theme]()
        {
            set_active_theme(theme);
        });
        register_themed(button_slot);
    };

    configure_theme_button(
        _blue_glass_moon_theme_button,
        0.0f,
        428.0f,
        "ui_test_scene.theme_blue_glass_moon",
        elysia::ui::UiBuiltinTheme::BlueGlassMoon);
    configure_theme_button(
        _elysia_light_theme_button,
        152.0f,
        428.0f,
        "ui_test_scene.theme_elysia_light",
        elysia::ui::UiBuiltinTheme::ElysiaLight);
    configure_theme_button(
        _elysia_dark_theme_button,
        304.0f,
        428.0f,
        "ui_test_scene.theme_elysia_dark",
        elysia::ui::UiBuiltinTheme::ElysiaDark);
    configure_theme_button(
        _evangelion_unit_00_theme_button,
        456.0f,
        428.0f,
        "ui_test_scene.theme_evangelion_unit_00",
        elysia::ui::UiBuiltinTheme::EvangelionUnit00);
    configure_theme_button(
        _evangelion_unit_01_theme_button,
        76.0f,
        472.0f,
        "ui_test_scene.theme_evangelion_unit_01",
        elysia::ui::UiBuiltinTheme::EvangelionUnit01);
    configure_theme_button(
        _evangelion_unit_02_theme_button,
        228.0f,
        472.0f,
        "ui_test_scene.theme_evangelion_unit_02",
        elysia::ui::UiBuiltinTheme::EvangelionUnit02);
    configure_theme_button(
        _quiet_slate_theme_button,
        380.0f,
        472.0f,
        "ui_test_scene.theme_quiet_slate",
        elysia::ui::UiBuiltinTheme::QuietSlate);

    _root_window->set_child_layout_options(0,make_window_child_options(0.0f,0.0f));
    _root_window->set_child_layout_options(1,make_window_child_options(288.0f,0.0f));
    _root_window->set_child_layout_options(2,make_window_child_options(624.0f,0.0f));
    _root_window->set_child_layout_options(3,make_window_child_options(288.0f,220.0f));
    _root_window->set_child_layout_options(4,make_window_child_options(624.0f,300.0f));

    _root_window->register_focus_scope(*vertical_scroll,elysia::ui::UiFocusScopeNeighbors{ nullptr,hidden_scroll,nullptr,horizontal_scroll });
    _root_window->register_focus_scope(*horizontal_scroll,elysia::ui::UiFocusScopeNeighbors{ nullptr,hidden_scroll,vertical_scroll,grid_scroll });
    _root_window->register_focus_scope(*grid_scroll,elysia::ui::UiFocusScopeNeighbors{ nullptr,widget_scroll,horizontal_scroll,nullptr });
    _root_window->register_focus_scope(*hidden_scroll,elysia::ui::UiFocusScopeNeighbors{ vertical_scroll,nullptr,nullptr,nullptr });
    _root_window->register_focus_scope(*widget_scroll,elysia::ui::UiFocusScopeNeighbors{ grid_scroll,nullptr,hidden_scroll,nullptr });
    sync_theme_switch_button_roles();
    _root_window->focus_first_available_scope();
}

void UiContainerTestScene::clear_ui()
{
    _theme_registrations.clear();
    _blue_glass_moon_theme_button = nullptr;
    _elysia_light_theme_button = nullptr;
    _elysia_dark_theme_button = nullptr;
    _evangelion_unit_00_theme_button = nullptr;
    _evangelion_unit_01_theme_button = nullptr;
    _evangelion_unit_02_theme_button = nullptr;
    _quiet_slate_theme_button = nullptr;

    if (_root_window)
    {
        _root_window->destroy();
        _root_window = nullptr;
    }
}

void UiContainerTestScene::request_back_to_menu()
{
    std::cout << "ui container test back" << std::endl;
    request_scene_switch(AppSceneKeys::MainMenu);
}
}
