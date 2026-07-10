#include "main_menu_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/audio/audio_service.h"

#include "../../engine/ui/composites/ui_confirmation_dialog.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/layout/ui_layout_types.h"

#include <iostream>
#include <memory>

namespace arcneco::scene
{
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

    if (!_main_menu_window)
        return;

    _exit_confirmation = _main_menu_window->create_child<elysia::ui::UiConfirmationDialog>(
        elysia::core::Rect{ 0,0,420,240 });
    if (_exit_confirmation)
    {
        _exit_confirmation->set_config(elysia::ui::UiConfirmationDialogConfig{
            .title = elysia::ui::ui_text_key("menu_scene.exit_confirm.title"),
            .message = elysia::ui::ui_text_key("menu_scene.exit_confirm.message"),
            .confirm = elysia::ui::ui_text_key("menu_scene.exit"),
            .cancel = elysia::ui::ui_text_key("menu_scene.exit_confirm.cancel"),
            .close = elysia::ui::ui_text_key("menu_scene.exit_confirm.close"),
            .confirm_theme_role = elysia::ui::UiButtonThemeRole::Danger
        });
        _exit_confirmation->set_on_confirm([this]() { Scene::request_quit(); });
        _exit_confirmation->register_as_overlay(*_main_menu_window);
    }

    //create the inner button container
    std::unique_ptr<elysia::ui::UiListContainer> ui_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,300,400 });

    constexpr int button_wide = 250;
    constexpr int button_hight = 75;
    //start button
    std::unique_ptr<elysia::ui::UiButton> ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,button_wide,75 });
    ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.start"));
    ui_button->set_on_click([this] {Scene::request_scene_switch(AppSceneKeys::CharacterSelect);});
    ui_list->add_back(std::move(ui_button));

    //setting button
    ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,button_wide,75 });
    ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.settings"));
    ui_button->set_on_click([this] {Scene::request_scene_switch(AppSceneKeys::Setting);});
    ui_list->add_back(std::move(ui_button));

    //about button
    ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,button_wide,75 });
    ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.about"));
    ui_button->set_on_click([this] {Scene::request_scene_switch(AppSceneKeys::UiContainerTest);});
    ui_list->add_back(std::move(ui_button));

    //exit button
    ui_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,button_wide,75 });
    ui_button->set_text_content(elysia::ui::ui_text_key("menu_scene.exit"));
    ui_button->set_on_click([this]
        {
            if (_exit_confirmation)
                _exit_confirmation->open();
        });
    ui_list->add_back(std::move(ui_button));

    //Title
    std::unique_ptr<elysia::ui::UiLabel> ui_label =
        std::make_unique<elysia::ui::UiLabel>(elysia::core::Rect{ 0,0,600,120 },0,elysia::ui::ui_text_key("menu_scene.project_name"));
    ui_label->set_theme_role(elysia::ui::UiLabelThemeRole::Title);
    ui_label->set_typography_role(elysia::ui::UiTypographyRole::Title);
    ui_label->set_target_height(96.0f);
    ui_label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    ui_label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);


    std::unique_ptr<elysia::ui::UiListContainer> ui_outer_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,300,400 });
    ui_outer_list->add_front(std::move(ui_label));
    ui_outer_list->add_back(std::move(ui_list));

    //add list to window
    elysia::ui::UiLayoutChildOptions layout{ elysia::ui::UiLayoutAnchor::Center };
    elysia::ui::UiElement* list_added = _main_menu_window->add_child(std::move(ui_outer_list), layout);

    if (auto* list = dynamic_cast<elysia::ui::UiListContainer*>(list_added))
        _main_menu_window->register_focus_scope(*list);

    elysia::ui::UiWindowStyle window_style = _main_menu_window->style();
    window_style.draw_background = true;
    window_style.draw_border = false;
    _main_menu_window->set_style(window_style);

    _main_menu_window->set_on_cancel([this]{
        if (_exit_confirmation)
            _exit_confirmation->open();
        });
}

void MainMenuScene::reset_exit_overlay()
{
    if (_main_menu_window && _exit_confirmation && !_main_menu_window->is_destroyed() && !_exit_confirmation->is_destroyed())
        _exit_confirmation->close();
}

void MainMenuScene::restore_menu_state()
{
    if (!_main_menu_window || _main_menu_window->is_destroyed())
        return;

    _main_menu_window->set_active(true);
    _main_menu_window->set_visible(true);

    if (_exit_confirmation && !_exit_confirmation->is_destroyed())
        _exit_confirmation->close();

    _main_menu_window->focus_first_available_scope();
}

}

