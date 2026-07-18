#include "engine_feature_test_scene.h"

#include "../../assist/engine_assist_cache.h"
#include "../../core/render/colors.h"
#include "../../input/raw_input_types.h"
#include "../../ui/widgets/image/ui_animation.h"
#include "../../scene/runtime/scene_runtime_context.h"

#include <stdexcept>
#include <array>
#include <optional>

namespace elysia::testbed
{
namespace
{
bool is_valid_return_route(const elysia::scene::SceneRoute& route) noexcept
{
    return elysia::scene::SceneKeys::is_supported(route.target);
}

const std::array<std::optional<elysia::core::Color>,5> kColorOverlays = {
    std::nullopt,
    elysia::core::colors::white,
    elysia::core::colors::blue_500,
    elysia::core::colors::purple_500,
    elysia::core::colors::gray_700
};
}

void EngineFeatureTestScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    elysia::scene::Scene::on_input(input,events);
    for (const elysia::input::RawInputEvent& event : events)
    {
        if (event.control == elysia::input::RawInputControl::KeyEscape
            && event.type == elysia::input::RawInputEventType::ControlPressed)
        {
            return_to_caller();
            return;
        }
        if (event.control == elysia::input::RawInputControl::KeySpace
            && event.type == elysia::input::RawInputEventType::ControlPressed)
        {
            _color_overlay_index =
                (_color_overlay_index + 1) % kColorOverlays.size();
            apply_secondary_color_overlay();
        }
    }
}

void EngineFeatureTestScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    const TestbedScenePayload* test_payload =
        elysia::scene::try_scene_payload<TestbedScenePayload>(payload);
    if (!test_payload || !is_valid_return_route(test_payload->return_route))
        throw std::logic_error("EngineFeatureTestScene requires TestbedScenePayload with a valid return route.");

    _return_route = test_payload->return_route;
    _paused = false;
    const auto* cache = runtime_context().engine_assist_cache();
    if (!cache || !cache->initialized())
        throw std::logic_error("EngineFeatureTestScene requires an initialized EngineAssistCache.");
    if (!_primary_animation)
    {
        _primary_animation = create_and_add_object<elysia::ui::UiAnimation>(
            elysia::core::Rect{ 160.0f,200.0f,292.0f,292.0f });
        if (!_primary_animation->set_engine_animation(*cache,"engine.test.idle"))
            throw std::logic_error("EngineFeatureTestScene could not bind engine.test.idle.");
    }
    if (!_secondary_animation)
    {
        _secondary_animation = create_and_add_object<elysia::ui::UiAnimation>(
            elysia::core::Rect{ 760.0f,204.0f,324.0f,284.0f });
        if (!_secondary_animation->set_engine_animation(*cache,"engine.test.idle"))
            throw std::logic_error("EngineFeatureTestScene could not bind engine.test.idle.");
    }
    _primary_animation->play();
    _secondary_animation->play();
    apply_secondary_color_overlay();
}

void EngineFeatureTestScene::on_exit()
{
    _paused = false;
    if (_primary_animation)
        _primary_animation->pause();
    if (_secondary_animation)
        _secondary_animation->pause();
}

void EngineFeatureTestScene::reset()
{
    _paused = false;
    _return_route = {};
    if (_primary_animation)
        _primary_animation->destroy();
    if (_secondary_animation)
        _secondary_animation->destroy();
    _primary_animation = nullptr;
    _secondary_animation = nullptr;
    _color_overlay_index = 2;
}

std::size_t EngineFeatureTestScene::color_overlay_index() const noexcept
{
    return _color_overlay_index;
}

void EngineFeatureTestScene::apply_secondary_color_overlay()
{
    if (_secondary_animation)
    {
        _secondary_animation->set_color_overlay(
            kColorOverlays[_color_overlay_index]);
    }
}

void EngineFeatureTestScene::return_to_caller()
{
    if (is_valid_return_route(_return_route))
        request_scene_switch(_return_route);
}
}
