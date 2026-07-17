#pragma once

#include "../scene.h"
#include "engine_test_scene_payload.h"

namespace elysia::ui
{
class UiAnimation;
}

namespace elysia::scene::builtin
{
// Engine-owned playground for runtime gameplay features such as animation.
class EngineFeatureTestScene final : public Scene
{
public:
    void on_input(
        const elysia::input::RawInputFrame& input,
        const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void return_to_caller();

private:
    SceneRoute _return_route;
    elysia::ui::UiAnimation* _primary_animation = nullptr;
    elysia::ui::UiAnimation* _secondary_animation = nullptr;
};
}
