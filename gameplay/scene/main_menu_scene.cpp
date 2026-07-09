#include "main_menu_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/audio/audio_service.h"

#include "../../engine/ui/containers/ui_chrome_container.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/layout/ui_layout_types.h"

#include <array>
#include <iostream>
#include <memory>

namespace arcneco::scene
{
namespace
{
std::unique_ptr<elysia::ui::UiButton> make_menu_button(const elysia::core::Rect& rect,const char* text_key)
{
    auto button = std::make_unique<elysia::ui::UiButton>(rect);
    button->set_text_content(elysia::ui::ui_text_key(text_key));
    return button;
}

void close_overlay(elysia::ui::UiWindow* window,elysia::ui::UiElement* overlay)
{
    if (window && overlay)
        window->set_overlay_open(*overlay,false);
}
}

void MainMenuScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;

    if (_has_entered)
        return;

    if (elysia::audio::AudioService::instance()->play_music("scene.main_meun_scene_main"))
        std::cout << "play main meun music error" << std::endl;

    if (!_main_menu_window || _main_menu_window->is_destroyed())
        build_menu_buttons();

    restore_menu_state();

    _has_entered = true;
}

void MainMenuScene::on_update(double delta)
{
    elysia::scene::Scene::on_update(delta);
}

void MainMenuScene::on_render(SDL_Renderer* renderer)
{
    elysia::scene::Scene::on_render(renderer);
}

void MainMenuScene::on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events)
{
    ApplicationScene::on_input(input,events);
}

void MainMenuScene::on_exit()
{
    reset_exit_overlay();
}

void MainMenuScene::reset()
{
    _has_entered = false;
    reset_exit_overlay();
}

void MainMenuScene::build_menu_buttons()
{
    if (_main_menu_window && !_main_menu_window->is_destroyed())
        return;

    _main_menu_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{0,0,1280,720});
    _exit_confirmation = nullptr;

    if (_main_menu_window)
    {

        //exit pop up setting
        _exit_confirmation = _main_menu_window->create_child<elysia::ui::UiChromeContainer>(elysia::core::Rect{ 0,0,420,240 });
        if (_exit_confirmation)
        {
            _exit_confirmation->set_header_height(48.0f);
            _exit_confirmation->set_header_padding(elysia::ui::UiLayoutPadding{ 12.0f,6.0f,12.0f,6.0f });
            _exit_confirmation->set_body_padding(elysia::ui::UiLayoutPadding{ 20.0f,20.0f,20.0f,20.0f });

            auto title = std::make_unique<elysia::ui::UiLabel>(
                elysia::core::Rect{ 0,0,280,36 },0,
                "menu_scene.exit_confirm.title");
            title->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
            _exit_confirmation->add_title_child(std::move(title));

            //button X
            auto close_button = make_menu_button(elysia::core::Rect{ 0,0,36,36 },"menu_scene.exit_confirm.close");
            close_button->set_on_click([this] {close_overlay(_main_menu_window,_exit_confirmation);});
            _exit_confirmation->add_right_action(std::move(close_button));

            //contaner of popup
            auto body = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,380,152 });
            auto body_style = body->style();
            body_style.draw_border = false;
            body->set_style(body_style);

            auto message = std::make_unique<elysia::ui::UiLabel>(
                elysia::core::Rect{ 0,0,380,56 },
                0,
                "menu_scene.exit_confirm.message");
            message->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
            message->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
            body->add_child(std::move(message),elysia::ui::UiPanelInsertDirection::Down);

            //button cancel
            auto cancel_button = make_menu_button(elysia::core::Rect{ 0,72,180,56 },"menu_scene.exit_confirm.cancel");
            cancel_button->set_on_click([this]{ close_overlay(_main_menu_window, _exit_confirmation);});
            body->add_child(std::move(cancel_button),elysia::ui::UiPanelInsertDirection::Down);

            //button exit
            auto confirm_button = make_menu_button(elysia::core::Rect{ 200,72,180,56 },"menu_scene.exit");
            confirm_button->set_on_click([this]{Scene::request_quit();});
            body->add_child(std::move(confirm_button),elysia::ui::UiPanelInsertDirection::Right);

            _exit_confirmation->set_body(std::move(body));

            //overlay setting
            elysia::ui::UiOverlayOptions overlay_options;
            overlay_options.open = false;
            overlay_options.modal = true;
            overlay_options.close_on_cancel = true;
            overlay_options.close_on_outside_click = false;
            overlay_options.placement = elysia::ui::UiOverlayPlacement::Center;
            overlay_options.fallback_size = elysia::core::Vector2{ 420.0f,240.0f };
            overlay_options.order = 1000;
            _main_menu_window->register_overlay(*_exit_confirmation,overlay_options);
        }

        //create the main container
        std::unique_ptr<elysia::ui::UiListContainer> ui_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,250,500 });
        elysia::ui::UiLayoutChildOptions layout{ elysia::ui::UiLayoutAnchor::Center };

        //start button
        std::unique_ptr<elysia::ui::UiButton> ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.start"));
        ui_list->add_back(std::move(ui_button));

        //setting button
        ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.settings"));
        ui_button->set_on_click([this] {Scene::request_scene_switch(AppSceneKeys::UiContainerTest);});
        ui_list->add_back(std::move(ui_button));

        //about button
        ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_key("menu_scene.about");
        ui_button->set_on_click([this] {Scene::request_scene_switch(AppSceneKeys::UiContainerTest);});
        ui_list->add_back(std::move(ui_button));

        //exit button
        ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,200,75 });
        ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.exit"));
        ui_button->set_on_click([this]
        {
            if (_main_menu_window && _exit_confirmation)
                _main_menu_window->set_overlay_open(*_exit_confirmation,true);
        });
        ui_list->add_back(std::move(ui_button));

        //add list to window
        elysia::ui::UiElement* list_added = _main_menu_window->add_child(std::move(ui_list), layout);
        elysia::ui::UiWindowStyle window_style = _main_menu_window->style();
        window_style.draw_background = true;
        _main_menu_window->set_style(window_style);

        if (auto* list = dynamic_cast<elysia::ui::UiListContainer*>(list_added))
            _main_menu_window->register_focus_scope(*list);
    }
}

void MainMenuScene::reset_exit_overlay()
{
    if (_main_menu_window && _exit_confirmation && !_main_menu_window->is_destroyed() && !_exit_confirmation->is_destroyed())
        _main_menu_window->set_overlay_open(*_exit_confirmation,false);
}

void MainMenuScene::restore_menu_state()
{
    if (!_main_menu_window || _main_menu_window->is_destroyed())
        return;

    _main_menu_window->set_active(true);
    _main_menu_window->set_visible(true);

    if (_exit_confirmation && !_exit_confirmation->is_destroyed())
        _main_menu_window->set_overlay_open(*_exit_confirmation,false);

    _main_menu_window->focus_first_available_scope();
}

}

