#include "elysia_scene.h"

#include "../../assist/engine_assist_cache.h"
#include "../../input/raw_input_types.h"
#include "../../scene/runtime/scene_runtime_context.h"
#include "../../ui/widgets/image/ui_image.h"
#include "../../ui/widgets/label/ui_label.h"
#include "../../ui/window/ui_window.h"

#include <memory>
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

void ElysiaScene::on_input(const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    for (const elysia::input::RawInputEvent& event : events)
    {
        if (event.control == elysia::input::RawInputControl::KeyEscape
            && event.type == elysia::input::RawInputEventType::ControlPressed)
        {
            return_to_caller();
            return;
        }
    }

    elysia::scene::Scene::on_input(input,events);
}

void ElysiaScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    const TestbedScenePayload* testbed_payload =elysia::scene::try_scene_payload<TestbedScenePayload>(payload);
    if (!testbed_payload || !is_valid_return_route(testbed_payload->return_route))
    {
        throw std::logic_error(
            "ElysiaScene requires TestbedScenePayload with a valid return route.");
    }

    const auto* cache = runtime_context().engine_assist_cache();
    if (!cache || !cache->initialized())
        throw std::logic_error("ElysiaScene requires an initialized EngineAssistCache.");

    _return_route = testbed_payload->return_route;
    _paused = false;

    if (!_root_window || _root_window->is_destroyed())
        build_ui();

    _root_window->set_visible(true);
    _root_window->set_active(true);

}

void ElysiaScene::on_exit()
{
    _paused = false;
    if (_root_window && !_root_window->is_destroyed())
    {
        _root_window->set_active(false);
        _root_window->set_visible(false);
    }
}

void ElysiaScene::reset()
{
    _paused = false;
    _return_route = {};
    destroy_ui();
}

void ElysiaScene::build_ui()
{
    const auto* cache = runtime_context().engine_assist_cache();
    if (!cache)
        throw std::logic_error("ElysiaScene requires EngineAssistCache while building UI.");

    SDL_Texture* texture = cache->find_texture("engine.brand.elysia.default");
    if (!texture)
        throw std::logic_error("ElysiaScene requires engine.brand.elysia.default.");

    _root_window = create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 0,0,1280,720 },100);
    if (!_root_window)
        throw std::runtime_error("ElysiaScene could not create its UiWindow.");

    _root_window->set_on_cancel([this]() { return_to_caller(); });

    auto logo = std::make_unique<elysia::ui::UiImage>(texture,elysia::core::Rect{ 0,0,120,120 });
    _root_window->add_child(std::move(logo), elysia::ui::UiLayoutChildOptions{._anchor= elysia::ui::UiLayoutAnchor::TopCenter });

    auto title = std::make_unique<elysia::ui::UiLabel>(elysia::core::Rect{ 390,470,500,72 },0,elysia::ui::ui_raw_text("Elysia Engine"));
    title->set_visual_role(elysia::ui::UiLabelVisualRole::Title);
    title->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    _root_window->add_child(std::move(title));
}

void ElysiaScene::destroy_ui() noexcept
{
    if (_root_window)
        _root_window->destroy();
    _root_window = nullptr;
}

void ElysiaScene::return_to_caller()
{
    if (is_valid_return_route(_return_route))
        request_scene_switch(_return_route);
}
}
