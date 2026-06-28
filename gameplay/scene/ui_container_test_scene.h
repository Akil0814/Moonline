#pragma once

#include "../../application/scene/application_scene.h"

#include <vector>

namespace elysia::ui
{
class UiWindow;
}

namespace arcneco::scene
{
class UiContainerTestScene final : public ApplicationScene
{
public:
    UiContainerTestScene() = default;
    ~UiContainerTestScene() override = default;

    void on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void rebuild_ui();
    void clear_ui();
    void request_back_to_menu();

private:
    elysia::ui::UiWindow* _root_window = nullptr;
};
}
