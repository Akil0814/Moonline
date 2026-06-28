#include "main_menu_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"
#include "../../engine/audio/audio_service.h"
#include "../../engine/ui/widgets/ui_button.h"

#include <array>
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
constexpr std::size_t kUiContainersIndex = 1;
constexpr std::size_t kExitIndex = 3;

struct MenuDefinition
{
    const char* key = "";
    bool enabled = true;
};

constexpr std::array<MenuDefinition,4> kMenuDefinitions{{
    { "menu_scene.start",true },
    { "menu_scene.ui_containers",true },
    { "menu_scene.settings",false },
    { "menu_scene.exit",true }
}};
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
    ApplicationScene::on_input(input,events);
    (void)input;
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
    for (std::size_t index = 0; index < kMenuDefinitions.size(); ++index)
    {
        const MenuDefinition& definition = kMenuDefinitions[index];
        auto* button = elysia::scene::Scene::create_and_add_object<elysia::ui::UiButton>(
            elysia::core::Rect{
                static_cast<float>(kMenuCenterX) - (kMenuButtonWidth * 0.5f),
                static_cast<float>(current_y),
                kMenuButtonWidth,
                kMenuButtonHeight
            },
            elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ definition.key } },
            static_cast<int>(index));

        button->set_enabled(definition.enabled);
        button->set_focused(index == 0);
        button->set_text_point_size(kMenuTextPointSize);
        button->set_on_click([this,index]()
        {
            handle_menu_action(index);
        });

        _menu_button_entries.push_back(MenuButtonEntry{ button });
        current_y += kMenuVerticalSpacing;
    }
}

void MainMenuScene::clear_menu_buttons()
{
    for (MenuButtonEntry& entry : _menu_button_entries)
    {
        if (!entry.button)
            continue;
        entry.button->destroy();
        entry.button = nullptr;
    }

    _menu_button_entries.clear();
}

void MainMenuScene::handle_menu_action(std::size_t index)
{
    if (index == kUiContainersIndex)
    {
        request_scene_switch(AppSceneKeys::UiContainerTest);
        return;
    }
    if (index == kExitIndex)
    {
        request_quit();
        return;
    }

    std::cout << "MainMenuScene action pending: index=" << index << std::endl;
}
}
