#include "engine_feature_test_scene.h"

#include "../../assist/engine_assist_cache.h"
#include "../../input/raw_input_types.h"
#include "../../ui/widgets/image/ui_animation.h"
#include "../../scene/runtime/scene_runtime_context.h"

#include <stdexcept>

namespace elysia::testbed
{
namespace
{
bool is_valid_return_route(const elysia::scene::SceneRoute& route) noexcept
{
    return elysia::scene::SceneKeys::is_supported(route.target);
}
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
}

void EngineFeatureTestScene::return_to_caller()
{
    if (is_valid_return_route(_return_route))
        request_scene_switch(_return_route);
}
}
