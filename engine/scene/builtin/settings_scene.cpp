#include "settings_scene.h"

#include "../../config/user_config_service.h"
#include "../../localization/localization_manager.h"
#include "../../tools/logger.h"
#include "../../ui/layout/ui_layout_types.h"
#include "../../ui/presets/settings_panel.h"
#include "../../ui/window/ui_window.h"
#include "../scene_runtime_context.h"

#include <SDL.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace elysia::scene::builtin
{
namespace
{
bool is_valid_route(const SceneRoute& route) noexcept
{
    return SceneKeys::is_supported(route.target);
}

elysia::ui::SettingsPanelDraft make_draft(
    const elysia::bootstrap::UserConfigData& settings)
{
    return elysia::ui::SettingsPanelDraft{
        .resolution = {
            settings.window_width,
            settings.window_height
        },
        .fullscreen = settings.fullscreen,
        .master_volume = settings.audio.master_volume,
        .music_volume = settings.audio.music_volume,
        .sound_volume = settings.audio.sound_volume,
        .language = settings.language
    };
}

elysia::ui::SettingsPanelOptions make_panel_options(
    const elysia::bootstrap::UserConfigData& settings)
{
    std::vector<elysia::ui::SettingsResolution> resolutions;
    const int display_mode_count = SDL_GetNumDisplayModes(0);
    if (display_mode_count > 0)
    {
        resolutions.reserve(static_cast<std::size_t>(display_mode_count) + 1u);
        for (int index = 0; index < display_mode_count; ++index)
        {
            SDL_DisplayMode mode{};
            if (SDL_GetDisplayMode(0,index,&mode) == 0)
                resolutions.push_back({ mode.w,mode.h });
        }
    }
    resolutions.push_back({ settings.window_width,settings.window_height });

    const auto& supported_languages =
        elysia::localization::LocalizationManager::instance()
            ->supported_languages();
    return elysia::ui::SettingsPanelOptions{
        .resolutions = std::move(resolutions),
        .languages = supported_languages
    };
}

std::string describe_failure(const elysia::config::UserConfigFailure& failure)
{
    if (!failure.message.empty())
        return failure.message;
    if (!failure.setting_name.empty())
        return "Failed to apply setting: " + failure.setting_name;
    return "Failed to apply settings.";
}
}

void SettingsScene::on_enter(const ScenePayload& payload)
{
    const SettingsScenePayload* settings_payload =
        try_scene_payload<SettingsScenePayload>(payload);
    if (!settings_payload)
    {
        throw std::logic_error(
            "SettingsScene requires SettingsScenePayload.");
    }
    if (!is_valid_route(settings_payload->return_route))
    {
        throw std::logic_error(
            "SettingsScene requires a valid return route.");
    }

    auto* config_service = elysia::config::UserConfigService::instance();
    if (!config_service->is_initialized())
    {
        throw std::logic_error(
            "SettingsScene requires an initialized UserConfigService.");
    }

    _return_route = settings_payload->return_route;
    _baseline_state = config_service->user_config().runtime_state();
    _transitioning = false;
    _paused = false;

    if (!_window || _window->is_destroyed())
        build_ui();
    restore_ui_state();
}

void SettingsScene::on_exit()
{
    _paused = false;
    _transitioning = false;
    if (_settings_panel)
        _settings_panel->unregister_from_window();
    if (_window && !_window->is_destroyed())
    {
        _window->set_active(false);
        _window->set_visible(false);
    }
}

void SettingsScene::reset()
{
    _paused = false;
    _transitioning = false;
    destroy_ui();
    _return_route = {};
    _baseline_state = {};
}

void SettingsScene::build_ui()
{
    const float logical_width = static_cast<float>(
        std::max(1,runtime_context().logical_width()));
    const float logical_height = static_cast<float>(
        std::max(1,runtime_context().logical_height()));

    _window = create_and_add_object<elysia::ui::UiWindow>(
        elysia::core::Rect{ 0,0,logical_width,logical_height },
        10);
    if (!_window)
        throw std::runtime_error("SettingsScene could not create its UiWindow.");

    const float panel_width = std::min(700.0f,std::max(1.0f,logical_width - 32.0f));
    const float panel_height = std::min(680.0f,std::max(1.0f,logical_height - 32.0f));
    auto panel = std::make_unique<elysia::ui::SettingsPanel>(
        elysia::core::Rect{ 0,0,panel_width,panel_height });
    _settings_panel = panel.get();
    _settings_panel->set_on_save(
        [this](const elysia::ui::SettingsPanelDraft& draft)
        {
            save_draft(draft);
        });
    _settings_panel->set_on_back([this]() { return_to_caller(); });

    elysia::ui::UiElement* adopted = _window->add_child(
        std::move(panel),
        { elysia::ui::UiLayoutAnchor::Center });
    if (!adopted)
        throw std::runtime_error("SettingsScene could not adopt its SettingsPanel.");

    _window->register_focus_scope(*_settings_panel);
    _window->set_on_cancel([this]() { return_to_caller(); });
}

void SettingsScene::restore_ui_state()
{
    if (!_window || !_settings_panel || _window->is_destroyed())
        return;

    _settings_panel->set_draft(make_draft(_baseline_state.settings));
    _settings_panel->set_options(make_panel_options(_baseline_state.settings));
    _settings_panel->clear_status_message();
    _settings_panel->register_with_window(*_window);

    _window->set_visible(true);
    _window->set_active(true);
    _window->focus_first_available_scope();
}

void SettingsScene::save_draft(
    const elysia::ui::SettingsPanelDraft& draft)
{
    if (_transitioning || !_settings_panel)
        return;

    auto* config_service = elysia::config::UserConfigService::instance();
    elysia::bootstrap::UserConfigData requested =
        config_service->user_config().snapshot();
    requested.window_width = draft.resolution.width;
    requested.window_height = draft.resolution.height;
    requested.fullscreen = draft.fullscreen;
    requested.audio.master_volume = draft.master_volume;
    requested.audio.music_volume = draft.music_volume;
    requested.audio.sound_volume = draft.sound_volume;
    requested.language = draft.language;

    const auto result = config_service->apply_and_save_user_config(
        requested,
        _baseline_state);
    if (result)
    {
        _baseline_state = config_service->user_config().runtime_state();
        _settings_panel->set_draft(make_draft(_baseline_state.settings));
        _settings_panel->set_status_message("Settings saved.",false);
        return;
    }

    std::string message = describe_failure(result.error().cause);
    if (result.error().rollback_failure)
    {
        const std::string rollback_message =
            describe_failure(*result.error().rollback_failure);
        ELYSIA_LOG_ERROR(
            "settings",
            "Settings rollback failed: " << rollback_message);
        message += " Rollback failed: " + rollback_message;
    }

    // The service snapshot is authoritative after either a complete rollback
    // or a partial rollback failure.
    _baseline_state = config_service->user_config().runtime_state();
    _settings_panel->set_draft(make_draft(_baseline_state.settings));
    _settings_panel->set_status_message(std::move(message),true);
}

void SettingsScene::return_to_caller()
{
    if (_transitioning)
        return;

    _transitioning = true;
    if (_settings_panel)
        _settings_panel->set_draft(make_draft(_baseline_state.settings));
    request_scene_switch(_return_route);
}

void SettingsScene::destroy_ui() noexcept
{
    if (_settings_panel)
        _settings_panel->unregister_from_window();
    if (_window)
        _window->destroy();
    _settings_panel = nullptr;
    _window = nullptr;
}
}
