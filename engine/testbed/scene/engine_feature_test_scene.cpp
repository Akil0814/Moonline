#include "engine_feature_test_scene.h"

#include "../../builtin/resources/builtin_asset_cache.h"
#include "../../builtin/audio/builtin_audio_player.h"
#include "../../builtin/resources/builtin_asset_keys.h"
#include "../../core/render/colors.h"
#include "../../input/raw_input_types.h"
#include "../../ui/containers/ui_list_container.h"
#include "../../ui/widgets/ui_button.h"
#include "../../ui/widgets/image/ui_animation.h"
#include "../../ui/window/ui_window.h"
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

std::unique_ptr<elysia::ui::UiButton> make_audio_button(const char* label)
{
    auto button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 0,0,144,52 });
    button->set_text_content(elysia::ui::ui_raw_text(label));
    return button;
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
    const auto* cache = runtime_context().builtin_asset_cache();
    if (!cache || !cache->initialized())
        throw std::logic_error("EngineFeatureTestScene requires an initialized BuiltinAssetCache.");
    _audio_player = runtime_context().builtin_audio_player();
    if (!_audio_player || !_audio_player->bound())
    {
        throw std::logic_error(
            "EngineFeatureTestScene requires a bound BuiltinAudioPlayer.");
    }
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

    if (!_audio_window || _audio_window->is_destroyed())
        build_audio_controls();
    _audio_window->set_visible(true);
    _audio_window->set_active(true);
    _audio_window->focus_first_available_scope();
}

void EngineFeatureTestScene::on_exit()
{
    _paused = false;
    if (_primary_animation)
        _primary_animation->pause();
    if (_secondary_animation)
        _secondary_animation->pause();
    if (_audio_window && !_audio_window->is_destroyed())
    {
        _audio_window->set_active(false);
        _audio_window->set_visible(false);
    }
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
    destroy_audio_controls();
    _audio_player = nullptr;
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

void EngineFeatureTestScene::build_audio_controls()
{
    _audio_window = create_and_add_object<elysia::ui::UiWindow>(
        elysia::core::Rect{ 390.0f,530.0f,500.0f,120.0f },100);
    if (!_audio_window)
    {
        throw std::runtime_error(
            "EngineFeatureTestScene could not create its audio control window.");
    }

    auto controls = std::make_unique<elysia::ui::UiListContainer>(
        elysia::core::Rect{ 18.0f,24.0f,464.0f,52.0f });
    controls->set_direction(elysia::ui::UiListDirection::Horizontal);
    controls->set_item_spacing(16.0f);

    auto play_sound = make_audio_button("Play Sound");
    play_sound->set_on_click([this]()
    {
        if (_audio_player)
        {
            (void)_audio_player->play_sound(
                elysia::builtin::asset_keys::TestSound);
        }
    });
    controls->add_back(std::move(play_sound));

    auto play_music = make_audio_button("Play Music");
    play_music->set_on_click([this]()
    {
        if (_audio_player)
        {
            (void)_audio_player->play_music(elysia::builtin::asset_keys::TestMusic);
        }
    });
    controls->add_back(std::move(play_music));

    auto stop_music = make_audio_button("Stop Music");
    stop_music->set_on_click([this]()
    {
        if (_audio_player)
            _audio_player->stop_music();
    });
    controls->add_back(std::move(stop_music));

    elysia::ui::UiListContainer* controls_ptr = controls.get();
    _audio_window->add_child(std::move(controls));
    _audio_window->register_focus_scope(*controls_ptr);
}

void EngineFeatureTestScene::destroy_audio_controls() noexcept
{
    if (_audio_window)
        _audio_window->destroy();
    _audio_window = nullptr;
}

void EngineFeatureTestScene::return_to_caller()
{
    if (is_valid_return_route(_return_route))
        request_scene_switch(_return_route);
}
}
