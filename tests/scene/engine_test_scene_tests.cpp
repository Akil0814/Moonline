#define SDL_MAIN_HANDLED

#include "engine/assist/engine_assist_cache.h"
#include "engine/assist/engine_assist_catalog.h"
#include "engine/io/loaders/asset_config_types.h"
#include "engine/scene/builtin/engine_feature_test_scene.h"
#include "engine/scene/builtin/ui_test_scene.h"
#include "engine/scene/scene_manager.h"
#include "engine/scene/scene_runtime_context.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>

namespace
{
using moonline::tests::require;

class SdlFixture
{
public:
    SdlFixture()
    {
        require(SDL_Init(SDL_INIT_VIDEO) == 0,
            "Engine test scene tests must initialize SDL video");
        require((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == IMG_INIT_PNG,
            "Engine test scene tests must initialize PNG support");
        require(TTF_Init() == 0,
            "Engine test scene tests must initialize SDL_ttf");
        _surface = SDL_CreateRGBSurfaceWithFormat(0,1280,720,32,SDL_PIXELFORMAT_RGBA32);
        require(_surface != nullptr,
            "Engine test scene tests must create a software surface");
        _renderer = SDL_CreateSoftwareRenderer(_surface);
        require(_renderer != nullptr,
            "Engine test scene tests must create a software renderer");
    }

    ~SdlFixture()
    {
        SDL_DestroyRenderer(_renderer);
        SDL_FreeSurface(_surface);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    [[nodiscard]] SDL_Renderer* renderer() const noexcept { return _renderer; }

private:
    SDL_Surface* _surface = nullptr;
    SDL_Renderer* _renderer = nullptr;
};

struct ReturnPayload
{
    int marker = 0;
};

template <int Id>
class ReturnScene final : public elysia::scene::Scene
{
public:
    void on_enter(const elysia::scene::ScenePayload& payload) override
    {
        const ReturnPayload* route_payload =
            elysia::scene::try_scene_payload<ReturnPayload>(payload);
        if (!route_payload)
            throw std::logic_error("ReturnScene requires ReturnPayload.");
        marker = route_payload->marker;
    }

    void on_exit() override {}
    void reset() override {}

    static inline int marker = 0;
};

using FirstReturnScene = ReturnScene<1>;
using SecondReturnScene = ReturnScene<2>;

bool throws_logic_error_containing(
    const std::function<void()>& operation,
    std::string_view expected)
{
    try
    {
        operation();
    }
    catch (const std::logic_error& error)
    {
        return std::string(error.what()).find(expected) != std::string::npos;
    }
    return false;
}

void send_escape(elysia::scene::SceneManager& scene_manager)
{
    scene_manager.on_input(
        elysia::input::RawInputFrame{},
        { elysia::input::RawInputEvent{
            .control = elysia::input::RawInputControl::KeyEscape,
            .type = elysia::input::RawInputEventType::ControlPressed,
            .device = elysia::input::InputDevice::Keyboard
        } });
}

void test_payload_contract_names_each_scene()
{
    elysia::scene::builtin::UiTestScene ui_test_scene;
    require(throws_logic_error_containing(
            [&ui_test_scene] { ui_test_scene.on_enter({}); },
            "UiTestScene"),
        "UiTestScene must name itself when the Engine test payload is missing");

    elysia::scene::builtin::EngineFeatureTestScene feature_test_scene;
    const elysia::scene::ScenePayload invalid_payload =
        elysia::scene::builtin::EngineTestScenePayload{
            .return_route = elysia::scene::SceneRoute{ .target = 1000 }
        };
    require(throws_logic_error_containing(
            [&feature_test_scene,&invalid_payload] { feature_test_scene.on_enter(invalid_payload); },
            "EngineFeatureTestScene"),
        "EngineFeatureTestScene must name itself when the return route is invalid");
}

void test_escape_returns_the_full_caller_route()
{
    SdlFixture fixture;
    elysia::assist::EngineAssistCache cache;
    require(cache.initialize(
                fixture.renderer(),
                elysia::assist::EngineAssistCatalog(std::filesystem::path{ MOONLINE_SOURCE_DIR }))
                .has_value(),
        "Engine test scene tests must initialize Engine Assist resources");

    elysia::io::ContentRegistry registry;
    elysia::scene::SceneRuntimeContext context(
        fixture.renderer(),registry,1280,720,&cache);
    elysia::scene::SceneManager scene_manager;
    scene_manager.set_runtime_context(context);
    scene_manager.register_builtin_scene<elysia::scene::builtin::UiTestScene>(
        elysia::scene::builtin::UiTest);
    scene_manager.register_builtin_scene<elysia::scene::builtin::EngineFeatureTestScene>(
        elysia::scene::builtin::EngineFeatureTest);
    scene_manager.register_game_scene<FirstReturnScene>(1);
    scene_manager.register_game_scene<SecondReturnScene>(2);

    scene_manager.start(elysia::scene::SceneRoute{
        .target = elysia::scene::builtin::UiTest,
        .payload = elysia::scene::builtin::EngineTestScenePayload{
            .return_route = elysia::scene::SceneRoute{
                .target = 1,
                .payload = ReturnPayload{ .marker = 17 },
                .reload_mode = elysia::scene::SceneReloadMode::Reuse
            }
        }
    });
    send_escape(scene_manager);
    require(scene_manager.current_scene_key() == 1 && FirstReturnScene::marker == 17,
        "UiTestScene Escape must return the caller key and payload");

    scene_manager.on_scene_request(elysia::scene::SceneRequest{
        .type = elysia::scene::SceneRequestType::Switch,
        .route = elysia::scene::SceneRoute{
            .target = elysia::scene::builtin::EngineFeatureTest,
            .payload = elysia::scene::builtin::EngineTestScenePayload{
                .return_route = elysia::scene::SceneRoute{
                    .target = 2,
                    .payload = ReturnPayload{ .marker = 29 },
                    .reload_mode = elysia::scene::SceneReloadMode::Reset
                }
            }
        }
    });
    scene_manager.on_update(0.0);
    send_escape(scene_manager);
    require(scene_manager.current_scene_key() == 2 && SecondReturnScene::marker == 29,
        "EngineFeatureTestScene Escape must return the caller key and payload");

    scene_manager.shutdown();
    cache.shutdown();
}
}

int main()
{
    test_payload_contract_names_each_scene();
    test_escape_returns_the_full_caller_route();
    return EXIT_SUCCESS;
}
