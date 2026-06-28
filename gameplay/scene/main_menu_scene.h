#pragma once

#include "../../application/scene/application_scene.h"

#include "../../engine/ui/window/ui_window.h"



#include <cstddef>
#include <vector>

namespace elysia::ui
{
class UiButton;
}

namespace arcneco::scene
{
class MainMenuScene final : public ApplicationScene
{
public:
    MainMenuScene() = default;
    ~MainMenuScene() override = default;

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events) override;

    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:

    void rebuild_menu_buttons();
    void clear_menu_buttons();
    void handle_menu_action(std::size_t index);

private:
    elysia::ui::UiWindow* _main_menu = nullptr;

};
}
