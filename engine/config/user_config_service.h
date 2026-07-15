#pragma once

#include "user_config_initialization_failure.h"
#include "user_config.h"
#include "user_config_store.h"
#include "../tools/singleton.h"

#include <expected>
#include <filesystem>

class Application;

namespace elysia::bootstrap { class Bootstrapper; }

namespace elysia::config
{
class UserConfigService final : public elysia::tools::Singleton<UserConfigService>
{
    friend elysia::tools::Singleton<UserConfigService>;
    friend class elysia::bootstrap::Bootstrapper;
    friend class ::Application;

public:
    [[nodiscard]] UserConfig& user_config() noexcept { return _user_config; }
    [[nodiscard]] const UserConfig& user_config() const noexcept { return _user_config; }
    [[nodiscard]] std::expected<void,UserConfigFailure> save_user_config();
    void register_user_config_change_handler(IUserConfigChangeHandler& handler) noexcept;
    void unregister_user_config_change_handler(IUserConfigChangeHandler& handler) noexcept;
    [[nodiscard]] bool is_initialized() const noexcept { return _initialized; }
    [[nodiscard]] std::expected<UserConfigLoadResult,UserConfigInitializationFailure> initialize(
        const elysia::bootstrap::UserConfigData& default_settings,
        const std::filesystem::path& user_config_path);
    void shutdown() noexcept;

private:
    UserConfigService() = default;
    UserConfig _user_config;
    UserConfigStore _user_config_store;
    std::filesystem::path _user_config_path;
    bool _initialized = false;
};
}
