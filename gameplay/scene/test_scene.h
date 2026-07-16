#pragma once

#include "../../engine/scene/scene.h"

namespace elysia::ui
{
class UiAnimation;
}

namespace arcneco::scene
{
class TestScene final : public elysia::scene::Scene
{
public:
    TestScene() = default;
    ~TestScene() override = default;

    void on_input(
        const elysia::input::RawInputFrame& input,
        const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    elysia::ui::UiAnimation* _test_animation = nullptr;
    elysia::ui::UiAnimation* _flying_demon_idle = nullptr;
};
}
