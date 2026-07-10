#include "character_select_scene.h"

#include "../character/character_manager.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/audio/audio_service.h"
#include "../../engine/resources/resource_manager.h"

#include "../../engine/ui/widgets/image/ui_animation.h"
#include "../../engine/ui/widgets/image/ui_image.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_button_group.h"
#include "../../engine/ui/containers/ui_scroll_container.h"
#include "../../engine/ui/composites/ui_confirmation_dialog.h"

#include <iostream>
#include <memory>

namespace arcneco::scene
{
    void  CharacterSelectScene::on_update(double delta){Scene::on_update(delta);}

    void  CharacterSelectScene::on_render(SDL_Renderer* renderer){Scene::on_render(renderer);}

    void  CharacterSelectScene::on_input( const elysia::input::RawInputFrame& input,
        const std::vector<elysia::input::RawInputEvent>& events){Scene::on_input(input, events);}

    void  CharacterSelectScene::on_enter(const elysia::scene::ScenePayload& payload)
    {
        (void)payload;

        if (!elysia::audio::AudioService::instance()->play_music("scene.character_select_scene_main"))
            std::cout << "play character_select music error" << std::endl;

        const bool should_build_ui = !_main_window || _main_window->is_destroyed();
        if (should_build_ui)
            build_buttons();

        auto character_manager = arcneco::character::CharacterManager::instance();
        if(character_manager->init())
        {
            std::vector<arcneco::character::CharacterPrototype> character_list =
                character_manager->get_character_prototype();

            for (const arcneco::character::CharacterPrototype& iter : character_list)
            {
                _character_keys.emplace_back(iter.id);
                std::cout << iter.id << std::endl;
            }
        }

        if (should_build_ui && _main_window && !_main_window->is_destroyed())
            build_character_list();
    }

    void  CharacterSelectScene::on_exit()
    {

    }

    void  CharacterSelectScene::reset()
    {
    }

    void CharacterSelectScene::build_buttons()
    {
        if (_main_window && !_main_window->is_destroyed())
            return;

        clear_character_visual_refs();
        _main_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 0,0,1280,720 });

        //popup
        _exit_confirmation = nullptr;
        build_popup();
        build_left_panel();
        build_right_panel();
        _main_window->set_on_cancel([this]{
            if (_exit_confirmation)
                _exit_confirmation->open();});

    }

    void CharacterSelectScene::build_popup()
    {
        if (!_main_window)
            return;

        _exit_confirmation = _main_window->create_child<elysia::ui::UiConfirmationDialog>(
            elysia::core::Rect{ 0,0,420,240 });
        if (!_exit_confirmation)
            return;

        _exit_confirmation->set_config(elysia::ui::UiConfirmationDialogConfig{
            .title = elysia::ui::ui_text_key("menu_scene.exit_confirm.title"),
            .message = elysia::ui::ui_text_key("menu_scene.exit_confirm.message"),
            .confirm = elysia::ui::ui_text_key("menu_scene.exit"),
            .cancel = elysia::ui::ui_text_key("menu_scene.exit_confirm.cancel"),
            .close = elysia::ui::ui_text_key("menu_scene.exit_confirm.close")
        });
        _exit_confirmation->set_on_confirm([this]() {
            Scene::request_scene_switch(AppSceneKeys::MainMenu);
        });
        _exit_confirmation->register_as_overlay(*_main_window);
    }

    void CharacterSelectScene::build_character_list()
    {
        auto* horizontal_scroll = _main_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,800,200 });
        if (!horizontal_scroll)
            return;

        horizontal_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Horizontal);
        horizontal_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
        horizontal_scroll->set_scroll_step(elysia::core::Vector2(36.0f, 36.0f));

        auto button_group = std::make_unique<elysia::ui::UiButtonGroup>(elysia::core::Rect{ 0,0,0,200 });
        button_group->set_direction(elysia::ui::UiListDirection::Horizontal);
        _character_button_keys.clear();
        button_group->set_on_selection_changed([this](std::optional<std::size_t> selected_index)
        {
                if (selected_index && *selected_index < _character_button_keys.size())
                {
                    _current_character_key = _character_button_keys[*selected_index];
                    elysia::audio::AudioService::instance()->play_sound(_current_character_key + ".selected");
                    on_character_change();
                }
        });

        constexpr int character_ui_width = 96;
        constexpr int character_ui_height = 192;

        for (const std::string& character_key : _character_keys)
        {
            for (int i = 0;i < 10;i++)
            {
                SDL_Texture* avatar_tex =
                    elysia::resources::ResourceManager::instance()->find_texture(character_key + ".selecting_icon");
                auto character_button = std::make_unique<elysia::ui::UiButton>(
                    elysia::core::Rect{ 0,0,character_ui_width,character_ui_height },
                    elysia::ui::UiButtonConfig{
                        .content = elysia::ui::UiButtonIconContent{ avatar_tex }
                    }
                );
                _character_button_keys.push_back(character_key);
                button_group->add_button(std::move(character_button));
            }

        }

        horizontal_scroll->set_content(std::move(button_group));
        _main_window->register_focus_scope(*horizontal_scroll);
    }

    void CharacterSelectScene::build_right_panel()
    {
        auto ui_character_selected_background = std::make_unique<elysia::ui::UiAnimation>("ryougi_shiki.idle", elysia::core::Rect{ 0,0,256,256 }, 0);
        _character_visuals.idle_preview = ui_character_selected_background.get();
        _main_window->add_child(std::move(ui_character_selected_background), { elysia::ui::UiLayoutAnchor::BottomRight });
    }

    void CharacterSelectScene::build_left_panel()
    {
        SDL_Texture* tex =
            elysia::resources::ResourceManager::instance()->find_texture("ryougi_shiki.full");
        SDL_Texture* name_texture =
            elysia::resources::ResourceManager::instance()->find_texture("ryougi_shiki.name");

        auto ui_character_stand = std::make_unique<elysia::ui::UiImage>(tex, elysia::core::Rect{0,0,384,384}, 10);
        auto ui_character_name = std::make_unique<elysia::ui::UiImage>(name_texture, elysia::core::Rect{0,0,256,32}, 20);
        auto ui_character_selected_background = std::make_unique<elysia::ui::UiAnimation>("ryougi_shiki.selected_background", elysia::core::Rect{ 0,0,512,512 },0);
        _character_visuals.full_portrait = ui_character_stand.get();
        _character_visuals.name_image = ui_character_name.get();
        _character_visuals.selected_background = ui_character_selected_background.get();
        _main_window->add_child(std::move(ui_character_stand), { elysia::ui::UiLayoutAnchor::BottomLeft });
        _main_window->add_child(std::move(ui_character_name), { elysia::ui::UiLayoutAnchor::CenterLeft });
        _main_window->add_child(std::move(ui_character_selected_background), { elysia::ui::UiLayoutAnchor::BottomLeft });
    }

    void CharacterSelectScene::build_character_detailed()
    {

    }

    void CharacterSelectScene::on_character_change()
    {
        refresh_character_visuals();
    }

    void CharacterSelectScene::refresh_character_visuals()
    {
        if (_current_character_key.empty())
            return;

        if (_character_visuals.full_portrait)
        {
            SDL_Texture* texture = elysia::resources::ResourceManager::instance()->find_texture(
                _current_character_key + ".full");
            _character_visuals.full_portrait->set_texture(texture);
            _character_visuals.full_portrait->set_visible(texture != nullptr);
        }

        if (_character_visuals.name_image)
        {
            SDL_Texture* texture = elysia::resources::ResourceManager::instance()->find_texture(
                _current_character_key + ".name");
            _character_visuals.name_image->set_texture(texture);
            _character_visuals.name_image->set_visible(texture != nullptr);
        }

        if (_character_visuals.selected_background)
        {
            const bool loaded = _character_visuals.selected_background->set_animation_key(
                _current_character_key + ".selected_background");
            _character_visuals.selected_background->set_visible(loaded);
            if (loaded)
                _character_visuals.selected_background->play();
        }

        if (_character_visuals.idle_preview)
        {
            const bool loaded = _character_visuals.idle_preview->set_animation_key(_current_character_key + ".idle");
            _character_visuals.idle_preview->set_visible(loaded);
            if (loaded)
                _character_visuals.idle_preview->play();
        }
    }

    void CharacterSelectScene::clear_character_visual_refs() noexcept
    {
        _character_visuals = CharacterVisualRefs{};
    }

}
