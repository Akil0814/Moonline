#pragma once

#include "config_initialization_failure.h"
#include "readonly_config.h"
#include "user_settings.h"
#include "user_settings_store.h"
#include "../tools/singleton.h"

#include <expected>
#include <filesystem>

class Application;

namespace elysia::bootstrap { class Bootstrapper; }

namespace elysia::config
{
class ConfigService final : public elysia::tools::Singleton<ConfigService>
{
    friend elysia::tools::Singleton<ConfigService>;
    friend class elysia::bootstrap::Bootstrapper;
    friend class ::Application;

public:
    [[nodiscard]] const ReadonlyConfig& readonly_config() const noexcept { return _readonly_config; }
    [[nodiscard]] UserSettings& user_settings() noexcept { return _user_settings; }
    [[nodiscard]] const UserSettings& user_settings() const noexcept { return _user_settings; }
    [[nodiscard]] std::expected<void,UserSettingsFailure> save_user_settings();
    void register_settings_change_handler(ISettingsChangeHandler& handler) noexcept;
    void unregister_settings_change_handler(ISettingsChangeHandler& handler) noexcept;
    [[nodiscard]] bool is_initialized() const noexcept { return _initialized; }

private:
    ConfigService() = default;
    [[nodiscard]] std::expected<UserSettingsLoadResult,ConfigInitializationFailure> initialize(
        const elysia::bootstrap::RuntimeSettings& default_settings,
        const std::filesystem::path& user_settings_path,
        const std::filesystem::path& config_manifest_path);
    void shutdown() noexcept;

    ReadonlyConfig _readonly_config;
    UserSettings _user_settings;
    UserSettingsStore _user_settings_store;
    std::filesystem::path _user_settings_path;
    bool _initialized = false;
};
}
