#include "startup_failure_scene.h"

#include "../../tools/termination_manager.h"
#include "../../ui/composites/ui_confirmation_dialog.h"
#include "../../ui/window/ui_window.h"
#include "../scene_runtime_context.h"

#include <algorithm>
#include <stdexcept>

namespace elysia::scene::builtin
{
namespace
{
constexpr const char* fallback_diagnostic_message =
    "Startup resource loading failed.";
}

void StartupFailureScene::on_enter(const ScenePayload& payload)
{
    const StartupFailureScenePayload* failure_payload =
        try_scene_payload<StartupFailureScenePayload>(payload);
    if (!failure_payload)
    {
        throw std::logic_error(
            "StartupFailureScene requires StartupFailureScenePayload.");
    }

    apply_payload(*failure_payload);
    _paused = false;

    if (!_window || _window->is_destroyed())
        build_ui();

    _dialog->set_config(make_dialog_config());
    _window->set_visible(true);
    _window->set_active(true);
    _dialog->open();
}

void StartupFailureScene::on_exit()
{
    _paused = false;
    if (_dialog)
        _dialog->close();
    if (_window && !_window->is_destroyed())
    {
        _window->set_active(false);
        _window->set_visible(false);
    }
}

void StartupFailureScene::reset()
{
    _paused = false;
    destroy_ui();
    _diagnostic_message.clear();
}

void StartupFailureScene::apply_payload(
    const StartupFailureScenePayload& payload)
{
    _diagnostic_message = payload.diagnostic_message.empty()
        ? fallback_diagnostic_message
        : payload.diagnostic_message;
}

void StartupFailureScene::build_ui()
{
    const float logical_width = static_cast<float>(
        std::max(1,runtime_context().logical_width()));
    const float logical_height = static_cast<float>(
        std::max(1,runtime_context().logical_height()));

    _window = create_and_add_object<elysia::ui::UiWindow>(
        elysia::core::Rect{ 0,0,logical_width,logical_height },
        10);
    if (!_window)
        throw std::runtime_error(
            "StartupFailureScene could not create its UiWindow.");

    const float dialog_width =
        std::min(520.0f,std::max(1.0f,logical_width - 32.0f));
    const float dialog_height =
        std::min(280.0f,std::max(1.0f,logical_height - 32.0f));
    _dialog = _window->create_child<elysia::ui::UiConfirmationDialog>(
        elysia::core::Rect{ 0,0,dialog_width,dialog_height },
        10);
    if (!_dialog)
    {
        throw std::runtime_error(
            "StartupFailureScene could not create its confirmation dialog.");
    }

    _dialog->set_config(make_dialog_config());
    _dialog->set_on_confirm([this]() { confirm_exit(); });
    if (!_dialog->register_with_window(*_window))
    {
        throw std::runtime_error(
            "StartupFailureScene could not register its confirmation dialog.");
    }
}

void StartupFailureScene::confirm_exit()
{
    elysia::tools::TerminationManager::instance()->request_termination(
        elysia::tools::TerminationReason::FatalRuntimeFailure,
        "startup",
        _diagnostic_message.empty()
            ? fallback_diagnostic_message
            : _diagnostic_message);
}

void StartupFailureScene::destroy_ui() noexcept
{
    if (_dialog)
        _dialog->unregister_from_window();
    if (_window)
        _window->destroy();
    _dialog = nullptr;
    _window = nullptr;
}

elysia::ui::UiConfirmationDialogConfig
StartupFailureScene::make_dialog_config()
{
    return elysia::ui::UiConfirmationDialogConfig{
        .title = elysia::ui::ui_text_key("engine.startup.failure.title"),
        .message = elysia::ui::ui_text_key("engine.startup.failure.message"),
        .confirm = elysia::ui::ui_text_key("engine.startup.failure.exit"),
        .cancel = elysia::ui::ui_text_key("engine.common.cancel"),
        .close = elysia::ui::ui_text_key("engine.common.close"),
        .confirm_visual_role = elysia::ui::UiButtonVisualRole::Danger
    };
}
}
