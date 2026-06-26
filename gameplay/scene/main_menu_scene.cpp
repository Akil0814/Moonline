#include "main_menu_scene.h"

#include "../../application/scene/scene_payloads.h"
#include "../../engine/audio/audio_service.h"
#include "../../engine/config/config_manager.h"
#include "../../engine/core/render/colors.h"
#include "../../engine/input/raw_input_types.h"
#include "../../engine/localization/localization_manager.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace arcneco::scene
{
namespace
{
constexpr int kMenuCenterX = 640;
constexpr int kMenuStartY = 220;
constexpr int kMenuVerticalSpacing = 70;
constexpr float kMenuButtonWidth = 320.0f;
constexpr float kMenuButtonHeight = 48.0f;
constexpr int kMenuTextPointSize = 24;
constexpr float kDemoPanelX = 860.0f;
constexpr float kDemoPanelY = 170.0f;
constexpr float kDemoPanelWidth = 300.0f;
constexpr float kDemoTitleHeight = 34.0f;
constexpr float kDemoSliderHeight = 86.0f;
constexpr float kDemoPreviewHeight = 42.0f;
constexpr float kDemoButtonHeight = 48.0f;
constexpr float kDemoSmallLabelHeight = 32.0f;
constexpr float kDemoGap = 14.0f;

const std::vector<std::string> kMenuKeys = {
    "menu_scene.start",
    "menu_scene.settings",
    "menu_scene.exit",
    "menu_scene.about"
};
}

void MainMenuScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    if (payload.has_value())
    {
        const MainMenuEnterPayload& enter_payload =
            elysia::scene::require_scene_payload<MainMenuEnterPayload>(payload);
        (void)enter_payload;
    }

    _paused = false;
    (void)elysia::audio::AudioService::instance()->play_music("scene.main_meun_scene_main");
    rebuild_menu_buttons();
    rebuild_ui_demo();
}

void MainMenuScene::on_update(double delta)
{
    elysia::scene::Scene::on_update(delta);
}

void MainMenuScene::on_render(SDL_Renderer* renderer)
{
    elysia::scene::Scene::on_render(renderer);
}

void MainMenuScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    ApplicationScene::on_input(input, events);
    (void)input;

    for (const elysia::input::RawInputEvent& event : events)
    {
        if (event.type != elysia::input::RawInputEventType::ControlPressed)
        {
            continue;
        }

        if (elysia::input::matches_control(elysia::input::RawInputControl::KeyF6, event.control))
        {
            cycle_language();
            continue;
        }

        if (elysia::input::matches_control(elysia::input::RawInputControl::KeyUp, event.control)
            || elysia::input::matches_control(elysia::input::RawInputControl::KeyW, event.control)
            || elysia::input::matches_control(elysia::input::RawInputControl::GamepadDPadUp, event.control))
        {
            move_focus(-1);
            continue;
        }

        if (elysia::input::matches_control(elysia::input::RawInputControl::KeyDown, event.control)
            || elysia::input::matches_control(elysia::input::RawInputControl::KeyS, event.control)
            || elysia::input::matches_control(elysia::input::RawInputControl::GamepadDPadDown, event.control))
        {
            move_focus(1);
        }
    }
}

void MainMenuScene::on_exit()
{
    _paused = false;
    clear_ui_demo();
    clear_menu_buttons();
}

void MainMenuScene::reset()
{
    _paused = false;
    clear_ui_demo();
    clear_menu_buttons();
}

void MainMenuScene::rebuild_menu_buttons()
{
    clear_menu_buttons();

    int current_y = kMenuStartY;
    for (size_t index = 0; index < kMenuKeys.size(); ++index)
    {
        const std::string& key = kMenuKeys[index];
        auto* button = elysia::scene::Scene::create_and_add_object<elysia::ui::UiButton>(
            elysia::core::Rect{
                static_cast<float>(kMenuCenterX) - (kMenuButtonWidth * 0.5f),
                static_cast<float>(current_y),
                kMenuButtonWidth,
                kMenuButtonHeight
            },
            elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ key } },
            static_cast<int>(index));

        if (index == 1)
            button->set_enabled(false);

        button->set_text_point_size(kMenuTextPointSize);
        button->set_on_click([key]()
        {
            std::cout << "MainMenuScene button clicked: " << key << std::endl;
        });

        _menu_button_entries.push_back(MenuButtonEntry{ key, button });
        current_y += kMenuVerticalSpacing;
    }

    set_focused_button(0);
}

void MainMenuScene::clear_menu_buttons()
{
    for (MenuButtonEntry& entry : _menu_button_entries)
    {
        if (entry.button)
        {
            entry.button->destroy();
            entry.button = nullptr;
        }
    }

    _menu_button_entries.clear();
    _focused_button_index = 0;
}

void MainMenuScene::rebuild_ui_demo()
{
    clear_ui_demo();

    int order = 100;
    float current_y = kDemoPanelY;
    _demo_title_label = elysia::scene::Scene::create_and_add_object<elysia::ui::UiLabel>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoTitleHeight),order++,"menu_scene.ui_demo");
    _demo_title_label->set_text_point_size(26);
    _demo_title_label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    _demo_title_label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
    current_y += kDemoTitleHeight + kDemoGap;

    _demo_slider = elysia::scene::Scene::create_and_add_object<elysia::ui::UiSlider>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoSliderHeight),
        elysia::ui::UiSliderConfig{
            .label_content = elysia::ui::UiSliderTextContent{ "menu_scene.ui_opacity" },
            .label_placement = elysia::ui::UiSliderLabelPlacement::Above,
            .value_label_mode = elysia::ui::UiSliderValueLabelMode::Value,
            .min_value = 0.0f,
            .max_value = 255.0f,
            .value = 255.0f,
            .step = 1.0f,
            .handle = elysia::ui::UiDragHandleConfig{
                .idle_color = elysia::core::colors::powder_blue,
                .focused_color = elysia::core::colors::white,
                .dragging_color = elysia::core::colors::white,
                .border_color = elysia::core::colors::sky_blue,
                .disabled_border_color = elysia::core::colors::gray_500
            },
            .bar_thickness = 8.0f,
            .value_target_height = 18.0f
        },
        order++);
    _demo_slider->set_background_color(elysia::core::colors::abyss_blue);
    _demo_slider->set_border_color(elysia::core::colors::sky_blue);
    _demo_slider->set_fill_color(elysia::core::colors::glacial_white);
    _demo_slider->set_on_value_changed([this](float value)
    {
        const int next_opacity = std::clamp(static_cast<int>(std::lround(value)),0,255);
        apply_demo_opacity(static_cast<std::uint8_t>(next_opacity));
    });
    current_y += kDemoSliderHeight + kDemoGap;

    _demo_preview_label = elysia::scene::Scene::create_and_add_object<elysia::ui::UiLabel>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoPreviewHeight),order++,"menu_scene.ui_preview");
    _demo_preview_label->set_draw_background(true);
    _demo_preview_label->set_background_color(elysia::core::colors::cobalt_blue);
    _demo_preview_label->set_text_color(elysia::core::colors::glacial_white);
    _demo_preview_label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    _demo_preview_label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
    _demo_preview_label->set_padding(6);
    current_y += kDemoPreviewHeight + kDemoGap;

    _demo_preview_button = elysia::scene::Scene::create_and_add_object<elysia::ui::UiButton>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoButtonHeight),
        elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ "menu_scene.ui_button" } },
        order++);
    _demo_preview_button->set_text_point_size(22);
    _demo_preview_button->set_idle_color(elysia::core::colors::royal_blue);
    _demo_preview_button->set_focused_color(elysia::core::colors::blue_700);
    _demo_preview_button->set_pushed_color(elysia::core::colors::midnight_blue);
    _demo_preview_button->set_border_color(elysia::core::colors::powder_blue);
    _demo_preview_button->set_on_click([]()
    {
        std::cout << "MainMenuScene UI demo button clicked." << std::endl;
    });
    current_y += kDemoButtonHeight + kDemoGap;

    _demo_blink_label = elysia::scene::Scene::create_and_add_object<elysia::ui::UiBlinkLabel>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoSmallLabelHeight),order++,"menu_scene.ui_blink");
    _demo_blink_label->set_text_color(elysia::core::colors::yellow_300);
    _demo_blink_label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    _demo_blink_label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
    _demo_blink_label->configure_playback(elysia::ui::effects::UiOpacityBlinkMode::VisibleFirst,0.0,0.4,0.4,std::nullopt);
    _demo_blink_label->play();
    current_y += kDemoSmallLabelHeight + 8.0f;

    _demo_pulse_label = elysia::scene::Scene::create_and_add_object<elysia::ui::UiPulseLabel>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoSmallLabelHeight),order++,"menu_scene.ui_pulse");
    _demo_pulse_label->set_text_color(elysia::core::colors::cyan_300);
    _demo_pulse_label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    _demo_pulse_label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
    _demo_pulse_label->configure_playback(elysia::ui::effects::UiOpacityPulseMode::MinToMax,0.0,0.9,0.9,std::nullopt,96,255);
    _demo_pulse_label->play();
    current_y += kDemoSmallLabelHeight + kDemoGap;

    _demo_hint_label = elysia::scene::Scene::create_and_add_object<elysia::ui::UiLabel>(
        elysia::core::Rect(kDemoPanelX,current_y,kDemoPanelWidth,kDemoSmallLabelHeight),order++,"menu_scene.ui_hint");
    _demo_hint_label->set_text_color(elysia::core::colors::gray_300);
    _demo_hint_label->set_text_point_size(18);
    _demo_hint_label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    _demo_hint_label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);

    apply_demo_opacity(255);
}

void MainMenuScene::clear_ui_demo()
{
    if (_demo_title_label)
    {
        _demo_title_label->destroy();
        _demo_title_label = nullptr;
    }
    if (_demo_slider)
    {
        _demo_slider->destroy();
        _demo_slider = nullptr;
    }
    if (_demo_preview_label)
    {
        _demo_preview_label->destroy();
        _demo_preview_label = nullptr;
    }
    if (_demo_preview_button)
    {
        _demo_preview_button->destroy();
        _demo_preview_button = nullptr;
    }
    if (_demo_blink_label)
    {
        _demo_blink_label->destroy();
        _demo_blink_label = nullptr;
    }
    if (_demo_pulse_label)
    {
        _demo_pulse_label->destroy();
        _demo_pulse_label = nullptr;
    }
    if (_demo_hint_label)
    {
        _demo_hint_label->destroy();
        _demo_hint_label = nullptr;
    }
}

void MainMenuScene::apply_demo_opacity(std::uint8_t opacity) noexcept
{
    if (_demo_title_label)
        _demo_title_label->set_opacity(opacity);
    if (_demo_preview_label)
        _demo_preview_label->set_opacity(opacity);
    if (_demo_preview_button)
        _demo_preview_button->set_opacity(opacity);
    if (_demo_hint_label)
        _demo_hint_label->set_opacity(opacity);
}

void MainMenuScene::cycle_language()
{
    elysia::localization::LocalizationManager* localization_manager = elysia::localization::LocalizationManager::instance();
    if (!localization_manager)
    {
        return;
    }

    const std::vector<std::string>& languages = localization_manager->supported_languages();
    if (languages.empty())
    {
        return;
    }

    const auto current_iterator = std::find(
        languages.begin(),
        languages.end(),
        localization_manager->current_language());

    size_t next_index = 0;
    if (current_iterator != languages.end())
    {
        next_index =
            (static_cast<size_t>(std::distance(languages.begin(), current_iterator)) + 1)
            % languages.size();
    }

    if (!localization_manager->set_language(languages[next_index]))
    {
        return;
    }

    elysia::config::ConfigManager* config_manager = elysia::config::ConfigManager::instance();
    std::string save_error;
    if (!config_manager->set_language(
        localization_manager->current_language(),
        save_error))
    {
        std::cout << "MainMenuScene warning: sync language config failed: "
            << save_error << std::endl;
    }
    else if (!config_manager->save(save_error))
    {
        std::cout << "MainMenuScene warning: save language failed: "
            << save_error << std::endl;
    }
}

void MainMenuScene::set_focused_button(size_t index)
{
    if (_menu_button_entries.empty())
    {
        _focused_button_index = 0;
        return;
    }

    _focused_button_index = std::min(index, _menu_button_entries.size() - 1);
    for (size_t button_index = 0; button_index < _menu_button_entries.size(); ++button_index)
    {
        if (_menu_button_entries[button_index].button)
        {
            _menu_button_entries[button_index].button->set_focused(button_index == _focused_button_index);
        }
    }
}

void MainMenuScene::move_focus(int direction)
{
    if (_menu_button_entries.empty() || direction == 0)
    {
        return;
    }

    const int button_count = static_cast<int>(_menu_button_entries.size());
    const int current_index = static_cast<int>(_focused_button_index);
    const int wrapped_index = (current_index + direction + button_count) % button_count;
    set_focused_button(static_cast<size_t>(wrapped_index));
}
}
