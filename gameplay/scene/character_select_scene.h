#pragma once
#include "../../application/scene/application_scene.h"
#include "../../engine/ui/window/ui_window.h"


#include <cstddef>
#include <string>
#include <vector>

namespace elysia::ui
{
class UiConfirmationDialog;
class UiAnimation;
class UiImage;
class UiButton;
class UiLabel;
class UiListContainer;
class UiPanel;
}

namespace arcneco::scene
{
    class  CharacterSelectScene final : public ApplicationScene
    {
    public:
        CharacterSelectScene() = default;
        ~CharacterSelectScene() override = default;

        void on_update(double delta) override;
        void on_render(SDL_Renderer* renderer) override;
        void on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events) override;

        void on_enter(const elysia::scene::ScenePayload& payload) override;
        void on_exit() override;
        void reset() override;

    private:    
        void build_ui();
        void build_character_list();
        void build_character_detailed();
        void build_action_buttons();
        void build_right_panel();
        void build_left_panel();
        void build_popup();

        void on_character_change();
        void refresh_character_visuals();
        void refresh_character_details();
        void set_character_visuals_visible(bool visible) noexcept;
        void set_character_details_visible(bool visible) noexcept;
        void clear_character_visual_refs() noexcept;
        void clear_character_detail_refs() noexcept;

    private:
        // Borrowed pointers to window-owned character visuals. They are cleared before rebuilding the window.
        struct CharacterVisualRefs
        {
            elysia::ui::UiImage* full_portrait = nullptr;
            elysia::ui::UiImage* name_image = nullptr;
            elysia::ui::UiAnimation* selected_background = nullptr;
            elysia::ui::UiAnimation* idle_preview = nullptr;
        };

        // Borrowed pointers to window-owned character detail and action UI.
        struct CharacterDetailRefs
        {
            elysia::ui::UiPanel* info_panel = nullptr;
            elysia::ui::UiLabel* title_label = nullptr;
            elysia::ui::UiListContainer* action_row = nullptr;
            elysia::ui::UiButton* confirm_button = nullptr;
            elysia::ui::UiButton* back_button = nullptr;
        };

        elysia::ui::UiWindow* _main_window = nullptr;
        elysia::ui::UiConfirmationDialog* _exit_confirmation = nullptr;

        CharacterVisualRefs _character_visuals{};
        CharacterDetailRefs _character_details{};

    private:

        std::string _current_character_key = {};
        std::vector<std::string> _character_keys = {};
        std::vector<std::string> _character_button_keys = {};
    };
}
