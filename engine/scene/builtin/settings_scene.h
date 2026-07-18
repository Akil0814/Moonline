#pragma once

#include "settings_scene_payload.h"
#include "../scene.h"
#include "../../config/user_config_data.h"
#include "../../config/user_config_types.h"

namespace elysia::ui
{
class SettingsPanel;
struct SettingsPanelDraft;
class UiWindow;
}

namespace elysia::scene::builtin
{
class SettingsScene final : public Scene
{
public:
    SettingsScene() = default;
    ~SettingsScene() override = default;

    void on_enter(const ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void build_ui();
    void restore_ui_state();
    void save_draft(const elysia::ui::SettingsPanelDraft& draft);
    void return_to_caller();
    void destroy_ui() noexcept;

private:
    SceneRoute _return_route;
    elysia::config::UserConfigRuntimeState _baseline_state;
    elysia::ui::UiWindow* _window = nullptr;
    elysia::ui::SettingsPanel* _settings_panel = nullptr;
    bool _transitioning = false;
};
}
