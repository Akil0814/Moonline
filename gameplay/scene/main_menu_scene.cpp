#include "main_menu_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/audio/audio_service.h"

#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/layout/ui_layout_types.h"

#include <array>
#include <iostream>

namespace arcneco::scene
{

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
    _main_menu_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{0,0,1280,720});
    if (_main_menu_window)
    {
        std::unique_ptr<elysia::ui::UiListContainer> ui_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,250,500 });
        elysia::ui::UiLayoutChildOptions layout{ elysia::ui::UiLayoutAnchor::Center };

        std::unique_ptr<elysia::ui::UiButton> ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_key("menu_scene.start");
        ui_list->add_back(std::move(ui_button));

        ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_key("menu_scene.settings");
        ui_button->set_on_click([this] {Scene::request_scene_switch(AppSceneKeys::UiContainerTest);});
        ui_list->add_back(std::move(ui_button));

        ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_key("menu_scene.exit");
        ui_button->set_on_click([this]{Scene::request_quit();});
        ui_list->add_back(std::move(ui_button));

        elysia::ui::UiElement* added = _main_menu_window->add_child(std::move(ui_list), layout);
        _main_menu_window->set_draw_background(true);

        if (auto* list = dynamic_cast<elysia::ui::UiListContainer*>(added))
            _main_menu_window->register_focus_scope(*list);
        _main_menu_window->focus_first_available_scope();

    }
}

void MainMenuScene::clear_menu_buttons()
{

//request_quit();

}

}

