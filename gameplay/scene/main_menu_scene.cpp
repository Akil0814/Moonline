#include "main_menu_scene.h"

#include "../../application/scene/scene_payloads.h"
#include "../../engine/audio/audio_service.h"
#include "../../engine/config/config_manager.h"
#include "../../engine/input/raw_input_types.h"
#include "../../engine/localization/localization_manager.h"

#include <algorithm>
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
    clear_menu_buttons();
}

void MainMenuScene::reset()
{
    _paused = false;
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
