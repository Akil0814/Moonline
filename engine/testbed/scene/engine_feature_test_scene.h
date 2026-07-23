#pragma once

#include "../../scene/scene.h"
#include "testbed_scene_payload.h"

#include <cstddef>

namespace elysia::ui
{
class UiAnimation;
class UiWindow;
}

namespace elysia::builtin
{
class BuiltinAudioPlayer;
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
    [[nodiscard]] std::size_t color_overlay_index() const noexcept;

private:
    void return_to_caller();
    void apply_secondary_color_overlay();
    void build_audio_controls();
    void destroy_audio_controls() noexcept;

private:
    elysia::scene::SceneRoute _return_route;
    elysia::ui::UiAnimation* _primary_animation = nullptr;
    elysia::ui::UiAnimation* _secondary_animation = nullptr;
    elysia::ui::UiWindow* _audio_window = nullptr;
    const elysia::builtin::BuiltinAudioPlayer* _audio_player = nullptr;
    std::size_t _color_overlay_index = 2;
};
}
