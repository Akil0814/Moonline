#include "character_select_scene.h"

#include "../character/character_manager.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/audio/audio_service.h"
#include "../../engine/resources/resource_manager.h"

#include "../../engine/ui/widgets/image/ui_animation.h"
#include "../../engine/ui/widgets/image/ui_image.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_button_group.h"
#include "../../engine/ui/containers/ui_panel.h"
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
            build_ui();

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

    void CharacterSelectScene::build_ui()
    {
        if (_main_window && !_main_window->is_destroyed())
            return;

        clear_character_visual_refs();
        clear_character_detail_refs();
        _main_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 0,0,1280,720 });

        SDL_Texture* tex =
            elysia::resources::ResourceManager::instance()->find_texture("ui.moon");
        auto ui_background = std::make_unique<elysia::ui::UiImage>(tex, elysia::core::Rect{ 0,0,1280,720 }, -10);
        _main_window->add_child(std::move(ui_background), { elysia::ui::UiLayoutAnchor::Center });

        //popup
        _exit_confirmation = nullptr;
        build_popup();
        build_left_panel();
        build_right_panel();
        build_character_detailed();
        build_action_buttons();
        set_character_visuals_visible(false);
        set_character_details_visible(false);
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
            Scene::request_scene_switch(AppSceneKeys::MainMenu, MainMeunEnterPayload{ .replay_theme_music = true });
        });
        _exit_confirmation->register_as_overlay(*_main_window);
    }

    void CharacterSelectScene::build_character_list()
    {
        const elysia::ui::UiLayoutChildOptions scroll_layout{
            ._anchor = elysia::ui::UiLayoutAnchor::TopCenter,
            ._margin = elysia::ui::UiLayoutMargin{ .top = 20.0f }
        };
        auto* horizontal_scroll = _main_window->create_child<elysia::ui::UiScrollContainer>(
            scroll_layout,
            elysia::core::Rect{ 0,0,800,200 });
        if (!horizontal_scroll)
            return;

        elysia::ui::UiScrollContainerStyleOverrides scroll_style{};
        scroll_style.draw_background = false;
        scroll_style.draw_border = false;
        horizontal_scroll->set_style_overrides(scroll_style);
        horizontal_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Horizontal);
        horizontal_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
        horizontal_scroll->set_scroll_step(elysia::core::Vector2(36.0f, 36.0f));

        auto button_group = std::make_unique<elysia::ui::UiButtonGroup>(elysia::core::Rect{ 0,0,0,200 });
        button_group->set_direction(elysia::ui::UiListDirection::Horizontal);
        button_group->set_auto_select_first(false);
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
                elysia::ui::UiButtonStyleOverrides button_style{};
                button_style.chrome.draw_background = false;
                character_button->set_style_overrides(button_style);
                _character_button_keys.push_back(character_key);
                button_group->add_button(std::move(character_button));
            }

        }

        horizontal_scroll->set_content(std::move(button_group));
        _main_window->register_focus_scope(*horizontal_scroll);
    }

    void CharacterSelectScene::build_right_panel()
    {
        auto ui_character_selected_background = std::make_unique<elysia::ui::UiAnimation>("ryougi_shiki.idle", elysia::core::Rect{ 0,0,512,512 }, 0);
        _character_visuals.idle_preview = ui_character_selected_background.get();
        _main_window->add_child(std::move(ui_character_selected_background), {
            ._anchor = elysia::ui::UiLayoutAnchor::BottomLeft,
            ._margin = elysia::ui::UiLayoutMargin{ .left = -64.0f, .bottom = -24.0f }
        });
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
        _main_window->add_child(std::move(ui_character_stand), { elysia::ui::UiLayoutAnchor::BottomRight });
        _main_window->add_child(std::move(ui_character_name), {
            ._anchor = elysia::ui::UiLayoutAnchor::BottomLeft,
        });
        _main_window->add_child(std::move(ui_character_selected_background), { elysia::ui::UiLayoutAnchor::BottomRight });
    }

    void CharacterSelectScene::build_character_detailed()
    {
        auto info_panel = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,420,250 });
        info_panel->set_visual_role(elysia::ui::UiPanelVisualRole::Dialog);

        auto title = std::make_unique<elysia::ui::UiLabel>(
            elysia::core::Rect{ 0,0,372,36 },0,elysia::ui::ui_raw_text("CHARACTER INFO"));
        title->set_visual_role(elysia::ui::UiLabelVisualRole::Title);
        title->set_typography_role(elysia::ui::UiTypographyRole::DialogTitle);
        _character_details.title_label = title.get();
        info_panel->add_child(std::move(title),{
            ._anchor = elysia::ui::UiLayoutAnchor::TopLeft,
            ._margin = elysia::ui::UiLayoutMargin{ .left = 24.0f,.top = 20.0f }
        });

        const auto add_section = [&info_panel](const char* text,float top)
        {
            auto label = std::make_unique<elysia::ui::UiLabel>(
                elysia::core::Rect{ 0,0,372,28 },0,elysia::ui::ui_raw_text(text));
            label->set_visual_role(elysia::ui::UiLabelVisualRole::Subtitle);
            info_panel->add_child(std::move(label),{
                ._anchor = elysia::ui::UiLayoutAnchor::TopLeft,
                ._margin = elysia::ui::UiLayoutMargin{ .left = 24.0f,.top = top }
            });
        };
        add_section("Character details will appear here.",72.0f);
        add_section("ATTRIBUTES  —  Coming soon",126.0f);
        add_section("SKILLS      —  Coming soon",176.0f);

        _character_details.info_panel = info_panel.get();
        _main_window->add_child(std::move(info_panel),{
            ._anchor = elysia::ui::UiLayoutAnchor::Center,
            ._margin = elysia::ui::UiLayoutMargin{ .top = 20.0f }
        });
    }

    void CharacterSelectScene::build_action_buttons()
    {
        auto action_row = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,336,52 });
        action_row->set_direction(elysia::ui::UiListDirection::Horizontal);
        action_row->set_item_spacing(16.0f);

        auto confirm = std::make_unique<elysia::ui::UiButton>(
            elysia::core::Rect{ 0,0,160,52 },
            elysia::ui::UiButtonConfig{ .content = elysia::ui::ui_raw_text("CONFIRM") });
        confirm->set_visual_role(elysia::ui::UiButtonVisualRole::Primary);
        confirm->set_on_click([this]()
        {
            std::cout << "Character selection confirmed: " << _current_character_key << std::endl;
        });
        _character_details.confirm_button = confirm.get();
        action_row->add_back(std::move(confirm));

        auto back = std::make_unique<elysia::ui::UiButton>(
            elysia::core::Rect{ 0,0,160,52 },
            elysia::ui::UiButtonConfig{ .content = elysia::ui::ui_raw_text("BACK") });
        back->set_on_click([this]()
        {
            if (_exit_confirmation)
                _exit_confirmation->open();
        });
        _character_details.back_button = back.get();
        action_row->add_back(std::move(back));

        _character_details.action_row = action_row.get();
        elysia::ui::UiElement* added = _main_window->add_child(std::move(action_row),{
            ._anchor = elysia::ui::UiLayoutAnchor::BottomCenter,
            ._margin = elysia::ui::UiLayoutMargin{ .bottom = 24.0f }
        });
        if (auto* scope = dynamic_cast<elysia::ui::UiListContainer*>(added))
            _main_window->register_focus_scope(*scope);
    }

    void CharacterSelectScene::on_character_change()
    {
        refresh_character_visuals();
        refresh_character_details();
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

    void CharacterSelectScene::set_character_visuals_visible(bool visible) noexcept
    {
        if (_character_visuals.full_portrait)
            _character_visuals.full_portrait->set_visible(visible);
        if (_character_visuals.name_image)
            _character_visuals.name_image->set_visible(visible);
        if (_character_visuals.selected_background)
            _character_visuals.selected_background->set_visible(visible);
        if (_character_visuals.idle_preview)
            _character_visuals.idle_preview->set_visible(visible);
    }

    void CharacterSelectScene::refresh_character_details()
    {
        if (_current_character_key.empty())
            return;

        if (_character_details.title_label)
            _character_details.title_label->set_text_content(
                elysia::ui::ui_raw_text("CHARACTER INFO — " + _current_character_key));
        set_character_details_visible(true);
    }

    void CharacterSelectScene::set_character_details_visible(bool visible) noexcept
    {
        if (_character_details.info_panel)
            _character_details.info_panel->set_visible(visible);
        if (_character_details.confirm_button)
        {
            _character_details.confirm_button->set_visible(visible);
            _character_details.confirm_button->set_enabled(visible);
        }
    }

    void CharacterSelectScene::clear_character_visual_refs() noexcept
    {
        _character_visuals = CharacterVisualRefs{};
    }

    void CharacterSelectScene::clear_character_detail_refs() noexcept
    {
        _character_details = CharacterDetailRefs{};
    }

}
