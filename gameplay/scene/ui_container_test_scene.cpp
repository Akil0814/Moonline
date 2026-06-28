#include "ui_container_test_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../engine/core/render/colors.h"
#include "../../engine/input/raw_input_types.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_scroll_container.h"
#include "../../engine/ui/widgets/ui_bar.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/ui_slider.h"
#include "../../engine/ui/widgets/label/ui_blink_label.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/widgets/label/ui_pulse_label.h"
#include "../../engine/ui/widgets/number/ui_number.h"
#include "../../engine/ui/window/ui_window.h"

#include <iostream>
#include <memory>
#include <optional>

namespace arcneco::scene
{
namespace
{
constexpr float kRootX = 60.0f;
constexpr float kRootY = 54.0f;
constexpr float kRootWidth = 1160.0f;
constexpr float kRootHeight = 620.0f;
constexpr float kTitleHeight = 34.0f;
constexpr float kHintHeight = 26.0f;
constexpr float kTopButtonWidth = 132.0f;
constexpr float kTopButtonHeight = 38.0f;
constexpr float kBackButtonWidth = 120.0f;
constexpr float kBackButtonHeight = 38.0f;
constexpr float kPaneTop = 74.0f;
constexpr float kPaneWidth = 556.0f;
constexpr float kPaneHeight = 514.0f;
constexpr float kPaneTitleHeight = 28.0f;
constexpr float kScrollTop = 38.0f;
constexpr float kScrollHeight = 456.0f;
constexpr float kVerticalContentHeight = 900.0f;
constexpr float kHorizontalContentWidth = 1216.0f;
constexpr float kVerticalCardWidth = 500.0f;
constexpr float kVerticalCardHeight = 118.0f;
constexpr float kVerticalCardGap = 14.0f;
constexpr float kHorizontalCardWidth = 216.0f;
constexpr float kHorizontalCardHeight = 392.0f;
constexpr float kHorizontalCardGap = 16.0f;

[[nodiscard]] elysia::ui::UiLayoutMargin make_margin(float left,float top,float right = 0.0f,float bottom = 0.0f) noexcept
{
    return elysia::ui::UiLayoutMargin{ left,top,right,bottom };
}

[[nodiscard]] elysia::ui::UiLayoutChildOptions make_options(
    elysia::ui::UiLayoutAnchor anchor,
    const elysia::ui::UiLayoutMargin& margin = {},
    std::optional<elysia::core::Vector2> size_override = std::nullopt
) noexcept
{
    elysia::ui::UiLayoutChildOptions options{};
    options._anchor = anchor;
    options._margin = margin;
    if (size_override)
    {
        options._size_override = *size_override;
        options._use_size_override = true;
    }
    return options;
}

void style_title_label(elysia::ui::UiLabel* label,int point_size = 24)
{
    if (!label)
        return;
    label->set_text_point_size(point_size);
    label->set_text_color(elysia::core::colors::glacial_white);
    label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
}

void style_card_panel(elysia::ui::UiPanel* panel,elysia::core::Color background)
{
    if (!panel)
        return;
    panel->set_draw_background(true);
    panel->set_draw_border(true);
    panel->set_background_color(background);
    panel->set_border_color(elysia::core::colors::powder_blue);
    panel->set_padding(elysia::ui::UiLayoutPadding{ 8.0f,8.0f,8.0f,8.0f });
}

void style_body_label(elysia::ui::UiLabel* label,elysia::core::Color background = elysia::core::colors::transparent)
{
    if (!label)
        return;
    label->set_draw_background(background.a > 0);
    label->set_background_color(background);
    label->set_text_color(elysia::core::colors::glacial_white);
    label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
    label->set_padding(4);
}

void configure_slider(elysia::ui::UiSlider* slider,float value)
{
    if (!slider)
        return;
    slider->set_slider_config(elysia::ui::UiSliderConfig{
        .value_label_mode = elysia::ui::UiSliderValueLabelMode::Percent,
        .min_value = 0.0f,
        .max_value = 1.0f,
        .value = value,
        .step = 0.05f,
        .handle = elysia::ui::UiSliderHandleStyle{
            .idle_color = elysia::core::colors::powder_blue,
            .focused_color = elysia::core::colors::glacial_white,
            .dragging_color = elysia::core::colors::white,
            .border_color = elysia::core::colors::sky_blue,
            .disabled_border_color = elysia::core::colors::gray_500
        },
        .bar_thickness = 8.0f,
        .value_target_height = 16.0f
    });
    slider->set_draw_background(false);
    slider->set_draw_border(false);
    slider->set_fill_color(elysia::core::colors::glacial_white);
    slider->set_text_color(elysia::core::colors::white);
}

void build_vertical_demo(elysia::ui::UiPanel* pane)
{
    auto* title = pane->create_child<elysia::ui::UiLabel>(
        make_options(elysia::ui::UiLayoutAnchor::TopCenter,{},elysia::core::Vector2(kPaneWidth - 20.0f,kPaneTitleHeight)),
        elysia::core::Rect::zero(),0,"menu_scene.ui_vertical_scroll");
    style_title_label(title,22);

    auto* scroll = pane->create_child<elysia::ui::UiScrollContainer>(
        make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(0.0f,kScrollTop),elysia::core::Vector2(kPaneWidth - 20.0f,kScrollHeight)),
        elysia::core::Rect::zero(),0);
    scroll->set_padding(elysia::ui::UiLayoutPadding{ 8.0f,8.0f,8.0f,8.0f });
    scroll->set_scroll_step_y(38.0f);
    scroll->set_content_height(kVerticalContentHeight);

    auto content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect::zero(),0);
    auto* content_panel = content.get();
    content_panel->set_padding(elysia::ui::UiLayoutPadding{ 8.0f,8.0f,8.0f,8.0f });
    content_panel->set_direction(elysia::ui::UiListDirection::Vertical);
    content_panel->set_item_spacing(kVerticalCardGap);
    scroll->set_content(std::move(content));

    for (int index = 0; index < 6; ++index)
    {
        auto* card = content_panel->create_child<elysia::ui::UiPanel>(
            make_options(elysia::ui::UiLayoutAnchor::TopCenter,{},elysia::core::Vector2(kVerticalCardWidth,kVerticalCardHeight)),
            elysia::core::Rect::zero(),0);
        style_card_panel(card,index % 2 == 0 ? elysia::core::colors::royal_blue : elysia::core::colors::cobalt_blue);

        const char* title_key = index % 3 == 0 ? "menu_scene.ui_preview" : (index % 3 == 1 ? "menu_scene.ui_blink" : "menu_scene.ui_pulse");
        auto* card_title = card->create_child<elysia::ui::UiLabel>(
            make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(10.0f,8.0f),elysia::core::Vector2(188.0f,24.0f)),
            elysia::core::Rect::zero(),0,title_key);
        style_body_label(card_title);
        card_title->set_horizontal_align(elysia::ui::TextHorizontalAlign::Left);

        auto* number = card->create_child<elysia::ui::UiNumber>(
            make_options(elysia::ui::UiLayoutAnchor::TopRight,make_margin(0.0f,8.0f,10.0f,0.0f),elysia::core::Vector2(88.0f,24.0f)),
            elysia::core::Rect::zero(),0);
        number->set_value((index + 1) * 12.5);
        number->set_decimal_places(1);
        number->set_text_color(elysia::core::colors::glacial_white);
        number->set_horizontal_align(elysia::ui::TextHorizontalAlign::Right);
        number->set_vertical_align(elysia::ui::TextVerticalAlign::Center);

        auto* bar = card->create_child<elysia::ui::UiBar>(
            make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(10.0f,38.0f),elysia::core::Vector2(kVerticalCardWidth - 132.0f,12.0f)),
            elysia::core::Rect::zero(),0);
        bar->set_draw_border(true);
        bar->set_border_color(elysia::core::colors::powder_blue);
        bar->set_background_color(elysia::core::colors::abyss_blue);
        bar->set_fill_color(elysia::core::colors::glacial_white);
        bar->set_ratio(0.18f + static_cast<float>(index) * 0.12f);

        if (index % 3 == 0)
        {
            auto* slider = card->create_child<elysia::ui::UiSlider>(
                make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(10.0f,56.0f),elysia::core::Vector2(kVerticalCardWidth - 140.0f,42.0f)),
                elysia::core::Rect::zero(),0);
            configure_slider(slider,0.2f + static_cast<float>(index) * 0.08f);
        }
        else if (index % 2 == 0)
        {
            auto* blink = card->create_child<elysia::ui::UiBlinkLabel>(
                make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(10.0f,62.0f),elysia::core::Vector2(190.0f,24.0f)),
                elysia::core::Rect::zero(),0,"menu_scene.ui_blink");
            style_body_label(blink);
            blink->configure_playback(elysia::ui::effects::UiOpacityBlinkMode::VisibleFirst,0.0,0.35,0.35,std::nullopt);
            blink->play();
        }
        else
        {
            auto* pulse = card->create_child<elysia::ui::UiPulseLabel>(
                make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(10.0f,62.0f),elysia::core::Vector2(190.0f,24.0f)),
                elysia::core::Rect::zero(),0,"menu_scene.ui_pulse");
            style_body_label(pulse);
            pulse->configure_playback(elysia::ui::effects::UiOpacityPulseMode::MinToMax,0.0,0.9,0.9,std::nullopt,96,255);
            pulse->play();
        }

        auto* button = card->create_child<elysia::ui::UiButton>(
            make_options(elysia::ui::UiLayoutAnchor::BottomRight,make_margin(0.0f,0.0f,10.0f,10.0f),elysia::core::Vector2(118.0f,30.0f)),
            elysia::core::Rect::zero(),
            elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ "menu_scene.ui_button" } },
            0);
        button->set_text_point_size(18);
        button->set_on_click([index]()
        {
            std::cout << "UiContainerTestScene vertical button clicked: " << index << std::endl;
        });
    }
}

void build_horizontal_demo(elysia::ui::UiPanel* pane)
{
    auto* title = pane->create_child<elysia::ui::UiLabel>(
        make_options(elysia::ui::UiLayoutAnchor::TopCenter,{},elysia::core::Vector2(kPaneWidth - 20.0f,kPaneTitleHeight)),
        elysia::core::Rect::zero(),0,"menu_scene.ui_horizontal_scroll");
    style_title_label(title,22);

    auto* scroll = pane->create_child<elysia::ui::UiScrollContainer>(
        make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(0.0f,kScrollTop),elysia::core::Vector2(kPaneWidth - 20.0f,kScrollHeight)),
        elysia::core::Rect::zero(),0);
    scroll->set_padding(elysia::ui::UiLayoutPadding{ 8.0f,8.0f,8.0f,8.0f });
    scroll->set_scroll_step_x(54.0f);
    scroll->set_scroll_step_y(54.0f);
    scroll->set_content_width(kHorizontalContentWidth);

    auto content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect::zero(),0);
    auto* content_panel = content.get();
    content_panel->set_padding(elysia::ui::UiLayoutPadding{ 8.0f,8.0f,8.0f,8.0f });
    content_panel->set_direction(elysia::ui::UiListDirection::Horizontal);
    content_panel->set_item_spacing(kHorizontalCardGap);
    scroll->set_content(std::move(content));

    for (int index = 0; index < 5; ++index)
    {
        auto* card = content_panel->create_child<elysia::ui::UiPanel>(
            make_options(elysia::ui::UiLayoutAnchor::Center,{},elysia::core::Vector2(kHorizontalCardWidth,kHorizontalCardHeight)),
            elysia::core::Rect::zero(),0);
        style_card_panel(card,index % 2 == 0 ? elysia::core::colors::cobalt_blue : elysia::core::colors::royal_blue);

        auto* card_title = card->create_child<elysia::ui::UiLabel>(
            make_options(elysia::ui::UiLayoutAnchor::TopCenter,make_margin(0.0f,10.0f),elysia::core::Vector2(kHorizontalCardWidth - 20.0f,26.0f)),
            elysia::core::Rect::zero(),0,index % 2 == 0 ? "menu_scene.ui_button" : "menu_scene.ui_preview");
        style_body_label(card_title);

        auto* number = card->create_child<elysia::ui::UiNumber>(
            make_options(elysia::ui::UiLayoutAnchor::TopCenter,make_margin(0.0f,44.0f),elysia::core::Vector2(96.0f,26.0f)),
            elysia::core::Rect::zero(),0);
        number->set_value(100.0 + static_cast<double>(index) * 7.0);
        number->set_decimal_places(0);
        number->set_text_color(elysia::core::colors::glacial_white);
        number->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
        number->set_vertical_align(elysia::ui::TextVerticalAlign::Center);

        auto* bar = card->create_child<elysia::ui::UiBar>(
            make_options(elysia::ui::UiLayoutAnchor::TopCenter,make_margin(0.0f,82.0f),elysia::core::Vector2(kHorizontalCardWidth - 24.0f,12.0f)),
            elysia::core::Rect::zero(),0);
        bar->set_draw_border(true);
        bar->set_border_color(elysia::core::colors::powder_blue);
        bar->set_background_color(elysia::core::colors::abyss_blue);
        bar->set_fill_color(index % 2 == 0 ? elysia::core::colors::glacial_white : elysia::core::colors::cyan_300);
        bar->set_ratio(0.24f + static_cast<float>(index) * 0.14f);

        auto* slider = card->create_child<elysia::ui::UiSlider>(
            make_options(elysia::ui::UiLayoutAnchor::TopCenter,make_margin(0.0f,108.0f),elysia::core::Vector2(kHorizontalCardWidth - 24.0f,56.0f)),
            elysia::core::Rect::zero(),0);
        configure_slider(slider,0.15f + static_cast<float>(index) * 0.12f);

        if (index % 2 == 0)
        {
            auto* blink = card->create_child<elysia::ui::UiBlinkLabel>(
                make_options(elysia::ui::UiLayoutAnchor::TopCenter,make_margin(0.0f,182.0f),elysia::core::Vector2(kHorizontalCardWidth - 24.0f,24.0f)),
                elysia::core::Rect::zero(),0,"menu_scene.ui_blink");
            style_body_label(blink,elysia::core::colors::blue_700);
            blink->configure_playback(elysia::ui::effects::UiOpacityBlinkMode::VisibleFirst,0.0,0.35,0.35,std::nullopt);
            blink->play();
        }
        else
        {
            auto* pulse = card->create_child<elysia::ui::UiPulseLabel>(
                make_options(elysia::ui::UiLayoutAnchor::TopCenter,make_margin(0.0f,182.0f),elysia::core::Vector2(kHorizontalCardWidth - 24.0f,24.0f)),
                elysia::core::Rect::zero(),0,"menu_scene.ui_pulse");
            style_body_label(pulse,elysia::core::colors::blue_700);
            pulse->configure_playback(elysia::ui::effects::UiOpacityPulseMode::MinToMax,0.0,0.9,0.9,std::nullopt,96,255);
            pulse->play();
        }

        auto* button = card->create_child<elysia::ui::UiButton>(
            make_options(elysia::ui::UiLayoutAnchor::BottomCenter,make_margin(0.0f,0.0f,0.0f,14.0f),elysia::core::Vector2(132.0f,34.0f)),
            elysia::core::Rect::zero(),
            elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ "menu_scene.ui_button" } },
            0);
        button->set_text_point_size(18);
        button->set_on_click([index]()
        {
            std::cout << "UiContainerTestScene horizontal button clicked: " << index << std::endl;
        });
    }
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
    for (const elysia::input::RawInputEvent& event : events)
    {
        if (event.type == elysia::input::RawInputEventType::ControlPressed
            && (event.control == elysia::input::RawInputControl::KeyEscape
                || event.control == elysia::input::RawInputControl::GamepadEast))
        {
            request_back_to_menu();
            break;
        }
    }
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

    _root_window = elysia::scene::Scene::create_and_add_object<elysia::ui::UiWindow>(
        elysia::core::Rect(kRootX,kRootY,kRootWidth,kRootHeight),
        100);
    _root_window->set_draw_background(true);
    _root_window->set_draw_border(true);
    _root_window->set_background_color(elysia::core::colors::abyss_blue);
    _root_window->set_border_color(elysia::core::colors::sky_blue);
    _root_window->set_padding(elysia::ui::UiLayoutPadding{ 16.0f,16.0f,16.0f,16.0f });
    _root_window->set_on_cancel([this]()
    {
        request_back_to_menu();
    });

    auto* title = _root_window->create_child<elysia::ui::UiLabel>(
        make_options(elysia::ui::UiLayoutAnchor::TopCenter,{},elysia::core::Vector2(kRootWidth - 200.0f,kTitleHeight)),
        elysia::core::Rect::zero(),0,"menu_scene.ui_containers");
    style_title_label(title,28);

    auto* hint = _root_window->create_child<elysia::ui::UiLabel>(
        make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(0.0f,38.0f),elysia::core::Vector2(kRootWidth - 320.0f,kHintHeight)),
        elysia::core::Rect::zero(),0,"menu_scene.ui_scroll_hint");
    style_body_label(hint);
    hint->set_horizontal_align(elysia::ui::TextHorizontalAlign::Left);

    auto* focus_button = _root_window->create_child<elysia::ui::UiButton>(
        make_options(elysia::ui::UiLayoutAnchor::TopLeft,{},elysia::core::Vector2(kTopButtonWidth,kTopButtonHeight)),
        elysia::core::Rect::zero(),
        elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ "menu_scene.ui_button" } },
        0);
    focus_button->set_text_point_size(20);
    focus_button->set_on_click([]()
    {
        std::cout << "UiWindow focus demo button clicked" << std::endl;
    });

    auto* back = _root_window->create_child<elysia::ui::UiButton>(
        make_options(elysia::ui::UiLayoutAnchor::TopRight,{},elysia::core::Vector2(kBackButtonWidth,kBackButtonHeight)),
        elysia::core::Rect::zero(),
        elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ "menu_scene.ui_back" } },
        0);
    back->set_text_point_size(20);
    back->set_on_click([this]()
    {
        request_back_to_menu();
    });

    auto* left_pane = _root_window->create_child<elysia::ui::UiPanel>(
        make_options(elysia::ui::UiLayoutAnchor::TopLeft,make_margin(0.0f,kPaneTop),elysia::core::Vector2(kPaneWidth,kPaneHeight)),
        elysia::core::Rect::zero(),0);
    left_pane->set_draw_background(true);
    left_pane->set_draw_border(true);
    left_pane->set_background_color(elysia::core::colors::cobalt_blue);
    left_pane->set_border_color(elysia::core::colors::powder_blue);
    left_pane->set_padding(elysia::ui::UiLayoutPadding{ 10.0f,10.0f,10.0f,10.0f });
    build_vertical_demo(left_pane);

    auto* right_pane = _root_window->create_child<elysia::ui::UiPanel>(
        make_options(elysia::ui::UiLayoutAnchor::TopRight,make_margin(0.0f,kPaneTop),elysia::core::Vector2(kPaneWidth,kPaneHeight)),
        elysia::core::Rect::zero(),0);
    right_pane->set_draw_background(true);
    right_pane->set_draw_border(true);
    right_pane->set_background_color(elysia::core::colors::cobalt_blue);
    right_pane->set_border_color(elysia::core::colors::powder_blue);
    right_pane->set_padding(elysia::ui::UiLayoutPadding{ 10.0f,10.0f,10.0f,10.0f });
    build_horizontal_demo(right_pane);

    _root_window->register_focus_target(*focus_button,elysia::ui::UiWindowFocusOptions{ .slot = elysia::ui::UiWindowFocusSlot::TopLeft });
    _root_window->register_focus_target(*back,elysia::ui::UiWindowFocusOptions{ .slot = elysia::ui::UiWindowFocusSlot::TopRight });
    _root_window->set_focus_neighbors(*focus_button,elysia::ui::UiWindowFocusNeighbors{ nullptr,nullptr,nullptr,back });
    _root_window->set_focus_neighbors(*back,elysia::ui::UiWindowFocusNeighbors{ nullptr,nullptr,focus_button,nullptr });
    _root_window->focus_first_available();
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
    request_scene_switch(AppSceneKeys::MainMenu);
}
}
