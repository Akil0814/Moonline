#pragma once

#include "../../application/scene/application_scene.h"

namespace arcneco::scene
{
class TestScene final : public ApplicationScene
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
};
}
