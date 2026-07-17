#pragma once

#include "../../scene/scene.h"
#include "testbed_scene_payload.h"

namespace elysia::ui
{
class UiAnimation;
}

namespace elysia::testbed
{
// Engine-owned playground for runtime gameplay features such as animation.
class EngineFeatureTestScene final : public elysia::scene::Scene
{
public:
    void on_input(
        const elysia::input::RawInputFrame& input,
        const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void return_to_caller();

private:
    elysia::scene::SceneRoute _return_route;
    elysia::ui::UiAnimation* _primary_animation = nullptr;
    elysia::ui::UiAnimation* _secondary_animation = nullptr;
};
}
