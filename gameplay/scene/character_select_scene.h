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
        void build_buttons();
        void build_character_list();
        void build_character_detailed();
        void build_right_panel();
        void build_left_panel();
        void build_popup();

        void on_character_change();
        void refresh_character_visuals();
        void set_character_visuals_visible(bool visible) noexcept;
        void clear_character_visual_refs() noexcept;

    private:

        elysia::ui::UiWindow* _main_window = nullptr;
        elysia::ui::UiConfirmationDialog* _exit_confirmation = nullptr;

        // Borrowed pointers to window-owned character visuals. They are cleared before rebuilding the window.
        struct CharacterVisualRefs
        {
            elysia::ui::UiImage* full_portrait = nullptr;
            elysia::ui::UiImage* name_image = nullptr;
            elysia::ui::UiAnimation* selected_background = nullptr;
            elysia::ui::UiAnimation* idle_preview = nullptr;
        };
        CharacterVisualRefs _character_visuals{};

    private:

        std::string _current_character_key = {};
        std::vector<std::string> _character_keys = {};
        std::vector<std::string> _character_button_keys = {};
    };
}
