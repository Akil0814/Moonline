#include "elysia_scene.h"

#include "../../assist/engine_assist_cache.h"
#include "../../assist/engine_assist_audio_player.h"
#include "../../assist/engine_assist_keys.h"

#include "../../input/raw_input_types.h"
#include "../../scene/runtime/scene_runtime_context.h"

#include "../../ui/widgets/image/ui_image.h"
#include "../../ui/widgets/image/ui_fade_image.h"
#include "../../ui/widgets/label/ui_label.h"

#include "../../ui/window/ui_window.h"
#include "../../ui/containers/ui_list_container.h"

#include <memory>
#include <stdexcept>

//test
#include <iostream>

namespace elysia::testbed
{
namespace
{
bool is_valid_return_route(const elysia::scene::SceneRoute& route) noexcept
{
    return elysia::scene::SceneKeys::is_supported(route.target);
}
}

void ElysiaScene::on_update(double delta)
{
    elysia::scene::Scene::on_update(delta);

    _code_timer.update(delta);
}

void ElysiaScene::on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events)
{
    for (const elysia::input::RawInputEvent& event : events)
    {
        if (event.control == elysia::input::RawInputControl::KeyEscape&& event.type == elysia::input::RawInputEventType::ControlPressed)
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

    _text_list.push_back("int main(int argc, char const *argv[])");
    _text_list.push_back("{");
    _text_list.push_back("    RestoreEgo();");
    _text_list.push_back("    RestorePurePinkHeart();");
    _text_list.push_back("    RestructureHerrscherOfHuman();");
    _text_list.push_back("    RestoreThirteenFlameChasers();");
    _text_list.push_back("    RebuildIncarnation();");
    _text_list.push_back("    return 0;");
    _text_list.push_back("}");


    const auto* cache = runtime_context().engine_assist_cache();
    if (!cache || !cache->initialized())
        throw std::logic_error("ElysiaScene requires an initialized EngineAssistCache.");

    _return_route = testbed_payload->return_route;
    _paused = false;


    const auto* audio_player = runtime_context().engine_assist_audio_player();
    if (!audio_player->play_music(elysia::assist::asset_keys::ElysianRealm))
        throw std::logic_error("ElysiaScene Play Music Error.");


    if (!_root_window || _root_window->is_destroyed())
        build_ui();
    _root_window->set_visible(true);
    _root_window->set_active(true);


    _code_timer.set_one_shot(false);
    _code_timer.set_wait_time(1);
    _code_timer.set_on_timeout([this]
        {
        add_lable(get_next_line());
        });

    _code_timer.restart();
}

void ElysiaScene::on_exit()
{
    const auto* audio_player = runtime_context().engine_assist_audio_player();
    audio_player->stop_music();
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

    auto logo = std::make_unique<elysia::ui::UiFadeImage>(texture, elysia::core::Rect{ 0,0,320,320 });
    logo->configure_playback(elysia::ui::effects::UiOpacityFadeMode::FadeInOut, 1.5, 1.5, 1.5);
    logo->play();
    logo->set_on_end([this] {_finish_logo = true;});
    _root_window->add_child(std::move(logo), elysia::ui::UiLayoutChildOptions{ ._anchor = elysia::ui::UiLayoutAnchor::Center });

    _code_list = _root_window->create_child<elysia::ui::UiListContainer>(
        elysia::ui::UiLayoutChildOptions{ ._anchor = elysia::ui::UiLayoutAnchor::BottomRight },
        elysia::core::Rect{0,0, 100,50 });
    _code_list->set_direction(elysia::ui::UiListDirection::Vertical);

    _elysia_theme.set_theme(elysia::ui::UiBuiltinTheme::ElysiaLight);
    _elysia_theme.register_root(*_root_window);
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

void ElysiaScene::add_lable(std::string code_line)
{
    if(_code_list==nullptr)
        throw std::runtime_error("ElysiaScene Code list is null.");

    auto lable = std::make_unique<elysia::ui::UiLabel>(elysia::core::Rect{0,0,100,50}, 0, elysia::ui::ui_raw_text("TEST"));
    _code_list->add_back(std::move(lable));
    _code_list->set_size(_code_list->content_extent());

    std::cout << "add code" << std::endl;
}

std::string ElysiaScene::get_next_line()
{
    if (_current_line > 9)
        return "";

    return _text_list[_current_line++];
}

}
