#include "main_menu_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/audio/audio_service.h"

#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_container_layout_types.h"





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
    clear_menu_buttons();
    _main_menu = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 100,100,500,500 }, 100);
    _main_menu->set_draw_background(true);
    _main_menu->set_draw_border(true);
    _main_menu->set_on_cancel([this]()
        {
            //pop up quit window;
            std::cout << "cancel" << std::endl;
        });

    const elysia::ui::UiLayoutChildOptions layout
    { elysia::ui::UiLayoutAnchor::Center };

    elysia::ui::UiListContainer* panel=_main_menu->create_child<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,300,900 });

    elysia::ui::UiButton* button = panel->create_child<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,50 });

    _main_menu->set_child_layout_options(1, layout);
    _main_menu->set_child_layout_options(0, layout);

    panel->create_child<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,50 });

}

void MainMenuScene::clear_menu_buttons()
{
    //request_scene_switch(AppSceneKeys::UiContainerTest);

//request_quit();

}

}
