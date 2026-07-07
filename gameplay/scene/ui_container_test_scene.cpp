#include "ui_container_test_scene.h"

#include "../../engine/ui/window/ui_window.h"
#include "../../engine/ui/containers/ui_grid_container.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_scroll_container.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/ui_checkbox.h"
#include "../../engine/ui/widgets/ui_labeled_checkbox.h"
#include "../../engine/ui/widgets/ui_slider.h"
#include "../../engine/ui/widgets/ui_text_input.h"
#include "../../engine/ui/layout/ui_layout_types.h"

#include <array>
#include <iostream>
#include <memory>
#include <utility>

namespace arcneco::scene
{
namespace
{
[[nodiscard]] elysia::ui::UiButtonConfig make_button_config(const char* text_key)
{
    return elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ text_key } };
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
    elysia::ui::UiCheckboxStyle style = checkbox->style();
    style.mark_style = mark_style;
    checkbox->set_style(style);
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
    auto checkbox = std::make_unique<elysia::ui::UiLabeledCheckbox>(rect,0);
    checkbox->set_text_key(text_key);
    checkbox->set_checked(checked);
    checkbox->set_label_placement(label_placement);
    checkbox->set_text_placement(text_placement);
    checkbox->set_draw_background(draw_background);
    checkbox->set_draw_border(draw_border);
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
    input->set_placeholder_text(std::move(placeholder_text));
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
    slider->set_text_key(text_key);
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

void UiContainerTestScene::rebuild_ui()
{
    clear_ui();

    _root_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 120,80,1040,560 },100);
    _root_window->set_draw_background(true);
    _root_window->set_draw_border(true);
    _root_window->set_padding(elysia::ui::UiLayoutPadding{ 24.0f,24.0f,24.0f,24.0f });
    _root_window->set_on_cancel([this]()
    {
        request_back_to_menu();
    });

    auto* vertical_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,260,420 });
    vertical_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    vertical_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    vertical_scroll->set_scroll_step(elysia::core::Vector2(32.0f,32.0f));
    vertical_scroll->set_draw_border(true);

    auto vertical_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,260,760 });
    vertical_list->set_padding(elysia::ui::UiLayoutPadding{ 20.0f,20.0f,20.0f,20.0f });
    vertical_list->set_item_spacing(18.0f);
    for (int index = 0; index < 10; ++index)
        vertical_list->add_back(make_button(index,"vertical"));
    vertical_scroll->set_content(std::move(vertical_list));

    auto* horizontal_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,180 });
    horizontal_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    horizontal_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    horizontal_scroll->set_scroll_step(elysia::core::Vector2(36.0f,36.0f));
    horizontal_scroll->set_draw_border(true);

    auto horizontal_panel = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,760,180 });
    horizontal_panel->set_draw_background(true);
    horizontal_panel->set_draw_border(true);

    auto horizontal_button_0 = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 70,68,120,44 },
        make_button_config(button_text_key_for_index(0)),
        0);
    horizontal_button_0->set_on_click([]()
    {
        std::cout << "horizontal button 0" << std::endl;
    });
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
        horizontal_panel->add_child(std::move(button),elysia::ui::UiPanelInsertDirection::Right);
    }
    horizontal_scroll->set_content(std::move(horizontal_panel));

    auto* grid_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,340,260 });
    grid_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    grid_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    grid_scroll->set_scroll_step(elysia::core::Vector2(28.0f,28.0f));
    grid_scroll->set_draw_border(true);

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
        grid_content->add_child(std::move(button));
    }
    grid_scroll->set_content(std::move(grid_content));

    auto* hidden_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,180 });
    hidden_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    hidden_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Hidden);
    hidden_scroll->set_scroll_step(elysia::core::Vector2(30.0f,30.0f));
    hidden_scroll->set_draw_border(true);

    auto first_hidden_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,320,360 });
    first_hidden_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    first_hidden_content->set_item_spacing(14.0f);
    for (int index = 0; index < 4; ++index)
        first_hidden_content->add_back(make_button(index,"hidden-initial"));
    hidden_scroll->set_content(std::move(first_hidden_content));

    auto replacement_hidden_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,320,520 });
    replacement_hidden_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    replacement_hidden_content->set_item_spacing(14.0f);
    for (int index = 0; index < 8; ++index)
        replacement_hidden_content->add_back(make_button(index,"hidden"));
    hidden_scroll->set_content(std::move(replacement_hidden_content));

    auto* widget_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,340,220 });
    widget_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    widget_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    widget_scroll->set_scroll_step(elysia::core::Vector2(24.0f,24.0f));
    widget_scroll->set_draw_border(true);

    auto widget_content = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,340,520 });
    widget_content->set_draw_background(true);
    widget_content->set_draw_border(true);

    widget_content->add_child(
        make_checkbox(
            elysia::core::Rect{ 18,18,36,36 },
            false,
            elysia::ui::UiCheckboxMarkStyle::Checkmark,
            "widget"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_checkbox(
            elysia::core::Rect{ 74,18,36,36 },
            true,
            elysia::ui::UiCheckboxMarkStyle::FilledBox,
            "widget"),
        elysia::ui::UiPanelInsertDirection::Right);
    widget_content->add_child(
        make_labeled_checkbox(
            elysia::core::Rect{ 18,74,220,40 },
            "menu_scene.settings",
            false,
            elysia::ui::UiLabeledCheckboxLabelPlacement::Right,
            elysia::ui::UiLabeledCheckboxTextPlacement::NearBox,
            false,
            false,
            "widget"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_labeled_checkbox(
            elysia::core::Rect{ 18,124,220,40 },
            "menu_scene.about",
            true,
            elysia::ui::UiLabeledCheckboxLabelPlacement::Right,
            elysia::ui::UiLabeledCheckboxTextPlacement::FarEdge,
            true,
            true,
            "widget"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_labeled_checkbox(
            elysia::core::Rect{ 18,174,220,40 },
            "menu_scene.start",
            false,
            elysia::ui::UiLabeledCheckboxLabelPlacement::Left,
            elysia::ui::UiLabeledCheckboxTextPlacement::NearBox,
            false,
            false,
            "widget"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_labeled_checkbox(
            elysia::core::Rect{ 18,224,220,40 },
            "menu_scene.exit",
            true,
            elysia::ui::UiLabeledCheckboxLabelPlacement::Left,
            elysia::ui::UiLabeledCheckboxTextPlacement::FarEdge,
            true,
            true,
            "widget"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_text_input(elysia::core::Rect{ 18,282,280,44 },"Type here",std::nullopt,"widget-main"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_text_input(elysia::core::Rect{ 18,336,280,44 },"Max 8 chars",std::optional<std::size_t>(8),"widget-limited"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_content->add_child(
        make_slider(elysia::core::Rect{ 18,394,280,78 },"menu_scene.start",0.42f,"widget"),
        elysia::ui::UiPanelInsertDirection::Down);
    widget_scroll->set_content(std::move(widget_content));

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
    _root_window->focus_first_available_scope();
}

void UiContainerTestScene::clear_ui()
{
    if (_root_window)
    {
        _root_window->destroy();
        _root_window = nullptr;
    }
}

void UiContainerTestScene::request_back_to_menu()
{
    std::cout << "ui container test back" << std::endl;
}
}
