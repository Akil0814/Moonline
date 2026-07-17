#define SDL_MAIN_HANDLED

#include "engine/scene/builtin/startup_loading_scene.h"
#include "engine/scene/scene_request_observer.h"
#include "engine/tools/termination_manager.h"
#include "tests/support/test_assertions.h"

#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>

namespace elysia::scene::builtin
{
class StartupLoadingSceneTestAccess
{
public:
    static void prime(
        StartupLoadingScene& scene,
        const StartupLoadingScenePayload& payload)
    {
        scene.clear_state();
        scene._startup_payload = payload;
        scene._completion.reset(payload.wait_for_confirmation);
    }

    static void finish_loading_then_intro(StartupLoadingScene& scene)
    {
        scene.mark_loading_finished();
        scene.mark_intro_finished();
    }

    static void fail(StartupLoadingScene& scene,std::string_view message)
    {
        scene.handle_failure(message);
    }
};
}

namespace
{
using moonline::tests::require;

struct RoutePayload
{
    int marker = 0;
};

class RequestProbe final : public elysia::scene::SceneRequestObserver
{
public:
    void on_scene_request(const elysia::scene::SceneRequest& value) override
    {
        request = value;
        ++request_count;
    }

    elysia::scene::SceneRequest request;
    int request_count = 0;
};

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

void test_payload_contract_names_startup_scene()
{
    elysia::scene::builtin::StartupLoadingScene scene;
    require(throws_logic_error_containing(
        [&scene]
        {
            scene.on_enter(elysia::scene::ScenePayload{ 42 });
        },
        "StartupLoadingScene"),
        "wrong startup payload type must fail with the built-in scene name");

    const elysia::scene::ScenePayload invalid_route =
        elysia::scene::builtin::StartupLoadingScenePayload{
            .success_route = elysia::scene::SceneRoute{ .target = 1000 }
        };
    require(throws_logic_error_containing(
        [&scene,&invalid_route] { scene.on_enter(invalid_route); },
        "StartupLoadingScene"),
        "invalid startup success route must fail with the built-in scene name");
}

void test_success_and_failure_routes_are_forwarded_unchanged()
{
    using namespace elysia::scene;
    using namespace elysia::scene::builtin;

    StartupLoadingScene success_scene;
    RequestProbe success_probe;
    success_scene.attach(&success_probe);
    StartupLoadingSceneTestAccess::prime(
        success_scene,
        StartupLoadingScenePayload{
            .success_route = SceneRoute{
                .target = 7,
                .payload = RoutePayload{ .marker = 71 },
                .reload_mode = SceneReloadMode::Reset
            },
            .wait_for_confirmation = false
        });
    StartupLoadingSceneTestAccess::finish_loading_then_intro(success_scene);
    const RoutePayload* success_payload =
        try_scene_payload<RoutePayload>(success_probe.request.route.payload);
    require(success_probe.request_count == 1
        && success_probe.request.type == SceneRequestType::Switch
        && success_probe.request.route.target == 7
        && success_probe.request.route.reload_mode == SceneReloadMode::Reset
        && success_payload && success_payload->marker == 71,
        "startup success must forward the complete success route");
    success_scene.detach(&success_probe);

    StartupLoadingScene failure_scene;
    RequestProbe failure_probe;
    failure_scene.attach(&failure_probe);
    StartupLoadingSceneTestAccess::prime(
        failure_scene,
        StartupLoadingScenePayload{
            .success_route = SceneRoute{ .target = 1 },
            .failure_route = SceneRoute{
                .target = 8,
                .payload = RoutePayload{ .marker = 82 },
                .reload_mode = SceneReloadMode::Recreate
            }
        });
    StartupLoadingSceneTestAccess::fail(
        failure_scene,
        "injected content failure");
    const RoutePayload* failure_payload =
        try_scene_payload<RoutePayload>(failure_probe.request.route.payload);
    require(failure_probe.request_count == 1
        && failure_probe.request.route.target == 8
        && failure_probe.request.route.reload_mode == SceneReloadMode::Recreate
        && failure_payload && failure_payload->marker == 82,
        "startup failure must forward the complete optional failure route");
    failure_scene.detach(&failure_probe);
}

void test_failure_without_route_requests_fatal_termination()
{
    using namespace elysia::scene;
    using namespace elysia::scene::builtin;
    auto* termination = elysia::tools::TerminationManager::instance();
    termination->reset_for_testing();

    StartupLoadingScene scene;
    StartupLoadingSceneTestAccess::prime(
        scene,
        StartupLoadingScenePayload{
            .success_route = SceneRoute{ .target = 1 }
        });
    StartupLoadingSceneTestAccess::fail(scene,"fatal startup failure");

    const auto info = termination->termination_info();
    require(info
        && info->reason == elysia::tools::TerminationReason::FatalRuntimeFailure
        && info->category == "startup"
        && info->message == "fatal startup failure",
        "startup failure without a route must request fatal termination");
    termination->reset_for_testing();
}
}

int main()
{
    test_payload_contract_names_startup_scene();
    test_success_and_failure_routes_are_forwarded_unchanged();
    test_failure_without_route_requests_fatal_termination();
    return EXIT_SUCCESS;
}
