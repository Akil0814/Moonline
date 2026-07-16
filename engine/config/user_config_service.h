#pragma once

#include "user_config.h"
#include "user_config_types.h"
#include "../tools/singleton.h"

#include <expected>
#include <filesystem>
#include <memory>
#include <string_view>

class Application;

namespace elysia::bootstrap { class Bootstrapper; }

namespace elysia::config
{
class IUserConfigChangeHandler
{
public:
    virtual ~IUserConfigChangeHandler() = default;

    virtual std::expected<void,UserConfigFailure> apply_master_volume(int value) = 0;
    virtual std::expected<void,UserConfigFailure> apply_music_volume(int value) = 0;
    virtual std::expected<void,UserConfigFailure> apply_sound_volume(int value) = 0;
    virtual std::expected<void,UserConfigFailure> apply_language(std::string_view language) = 0;
    virtual std::expected<void,UserConfigFailure> apply_target_fps(double value) = 0;
    virtual std::expected<void,UserConfigFailure> apply_window_size(int width,int height) = 0;
    virtual std::expected<void,UserConfigFailure> apply_fullscreen(bool value) = 0;
};

class UserConfigStore;
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
    UserConfigService();
    ~UserConfigService();
    UserConfig _user_config;
    std::unique_ptr<UserConfigStore> _user_config_store;
    std::filesystem::path _user_config_path;
    bool _initialized = false;
};
}
