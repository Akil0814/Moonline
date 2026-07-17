#define SDL_MAIN_HANDLED

#include "engine/scene/builtin/startup_failure_scene.h"
#include "engine/scene/scene_key.h"
#include "engine/tools/termination_manager.h"
#include "engine/ui/composites/ui_confirmation_dialog.h"
#include "tests/support/test_assertions.h"

#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace elysia::scene::builtin
{
class StartupFailureSceneTestAccess
{
public:
    static elysia::ui::UiConfirmationDialogConfig dialog_config()
    {
        return StartupFailureScene::make_dialog_config();
    }

    static void apply_payload(
        StartupFailureScene& scene,
        const StartupFailureScenePayload& payload)
    {
        scene.apply_payload(payload);
    }

    static void confirm_exit(StartupFailureScene& scene)
    {
        scene.confirm_exit();
    }

    static std::string_view diagnostic_message(
        const StartupFailureScene& scene)
    {
        return scene._diagnostic_message;
    }
};
}

namespace
{
using moonline::tests::require;
using elysia::scene::builtin::StartupFailureScene;
using elysia::scene::builtin::StartupFailureScenePayload;
using elysia::scene::builtin::StartupFailureSceneTestAccess;

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

void test_scene_identity_and_payload_contract()
{
    require(
        elysia::scene::builtin::StartupFailure == 0xFFFF0005u,
        "startup failure must reserve the next available built-in scene key");
    require(
        elysia::scene::SceneKeys::is_builtin(
            elysia::scene::builtin::StartupFailure),
        "startup failure key must belong to the built-in scene range");

    StartupFailureScene scene;
    require(
        throws_logic_error_containing(
            [&scene] { scene.on_enter(elysia::scene::ScenePayload{ 42 }); },
            "StartupFailureScene"),
        "wrong startup failure payload type must identify the built-in scene");
}

void test_dialog_uses_engine_localization_contract()
{
    const auto config = StartupFailureSceneTestAccess::dialog_config();
    require(
        config.title.kind == elysia::ui::UiTextContentKind::TextKey
            && config.title.value == "engine.startup.failure.title",
        "startup failure title must use the engine localization key");
    require(
        config.message.kind == elysia::ui::UiTextContentKind::TextKey
            && config.message.value == "engine.startup.failure.message",
        "startup failure message must use the engine localization key");
    require(
        config.confirm.kind == elysia::ui::UiTextContentKind::TextKey
            && config.confirm.value == "engine.startup.failure.exit",
        "startup failure exit action must use the engine localization key");
    require(
        config.cancel.value == "engine.common.cancel"
            && config.close.value == "engine.common.close",
        "startup failure dismiss actions must reuse common engine keys");
    require(
        config.confirm_visual_role == elysia::ui::UiButtonVisualRole::Danger,
        "startup failure exit action must use the danger visual role");
}

void test_confirm_publishes_diagnostic()
{
    auto* termination = elysia::tools::TerminationManager::instance();
    termination->reset_for_testing();

    StartupFailureScene scene;
    StartupFailureSceneTestAccess::apply_payload(
        scene,
        StartupFailureScenePayload{
            .diagnostic_message = "missing required texture"
        });
    StartupFailureSceneTestAccess::confirm_exit(scene);

    const auto info = termination->termination_info();
    require(
        info
            && info->reason
                == elysia::tools::TerminationReason::FatalRuntimeFailure
            && info->category == "startup"
            && info->message == "missing required texture",
        "confirming startup failure must publish its original diagnostic");
    termination->reset_for_testing();
}

void test_empty_diagnostic_uses_fallback()
{
    auto* termination = elysia::tools::TerminationManager::instance();
    termination->reset_for_testing();

    StartupFailureScene scene;
    StartupFailureSceneTestAccess::apply_payload(
        scene,
        StartupFailureScenePayload{});
    require(
        StartupFailureSceneTestAccess::diagnostic_message(scene)
            == "Startup resource loading failed.",
        "empty startup failure diagnostics must normalize to the fallback");

    StartupFailureSceneTestAccess::confirm_exit(scene);
    const auto info = termination->termination_info();
    require(
        info && info->message == "Startup resource loading failed.",
        "fallback diagnostic must be published on confirmation");
    termination->reset_for_testing();
}
}

int main()
{
    test_scene_identity_and_payload_contract();
    test_dialog_uses_engine_localization_contract();
    test_confirm_publishes_diagnostic();
    test_empty_diagnostic_uses_fallback();
    return EXIT_SUCCESS;
}
