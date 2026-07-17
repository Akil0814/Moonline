#pragma once

#include "startup_failure_scene_payload.h"
#include "../scene.h"

#include <string>

namespace elysia::ui
{
class UiConfirmationDialog;
class UiWindow;
struct UiConfirmationDialogConfig;
}

namespace elysia::scene::builtin
{
class StartupFailureScene final : public Scene
{
    friend class StartupFailureSceneTestAccess;

public:
    StartupFailureScene() = default;
    ~StartupFailureScene() override = default;

    void on_enter(const ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void apply_payload(const StartupFailureScenePayload& payload);
    void build_ui();
    void confirm_exit();
    void destroy_ui() noexcept;
    [[nodiscard]] static elysia::ui::UiConfirmationDialogConfig make_dialog_config();

private:
    std::string _diagnostic_message;
    elysia::ui::UiWindow* _window = nullptr;
    elysia::ui::UiConfirmationDialog* _dialog = nullptr;
};
}
