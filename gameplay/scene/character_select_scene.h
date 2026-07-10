#pragma once
#include "../../application/scene/application_scene.h"
#include "../../engine/ui/window/ui_window.h"


#include <cstddef>
#include <vector>

namespace elysia::ui
{
class UiConfirmationDialog;
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
        void build_popup();

    private:

        elysia::ui::UiWindow* _main_window = nullptr;
        elysia::ui::UiConfirmationDialog* _exit_confirmation = nullptr;

        std::string _current_character_key = {};
        std::vector<std::string> _character_keys = {};
    };
}
