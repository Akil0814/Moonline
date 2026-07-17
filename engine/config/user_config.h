#pragma once

#include "user_config_types.h"

#include <expected>
#include <string>
#include <string_view>

namespace elysia::config
{
class IUserConfigChangeHandler;

class UserConfig
{
public:
    [[nodiscard]] const elysia::bootstrap::WindowSettings&
        window_settings() const noexcept;
    [[nodiscard]] double target_fps() const noexcept;
    [[nodiscard]] bool vsync() const noexcept;
    [[nodiscard]] int master_volume() const noexcept;
    [[nodiscard]] int music_volume() const noexcept;
    [[nodiscard]] int sound_volume() const noexcept;
    [[nodiscard]] std::string_view language() const noexcept;

    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure>
        set_window_settings(const elysia::bootstrap::WindowSettings& settings);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_target_fps(double value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_vsync(bool value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_master_volume(int value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_music_volume(int value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_sound_volume(int value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_language(std::string value);

    [[nodiscard]] elysia::bootstrap::UserConfigData snapshot() const;
    [[nodiscard]] UserConfigRuntimeState runtime_state() const;
    [[nodiscard]] bool is_dirty() const noexcept;
    [[nodiscard]] bool restart_required() const noexcept;

private:
    friend class UserConfigService;
    void initialize(const elysia::bootstrap::UserConfigData& settings) noexcept;
    void mark_persisted() noexcept;
    void reset() noexcept;
    void register_change_handler(IUserConfigChangeHandler& handler) noexcept;
    void unregister_change_handler(IUserConfigChangeHandler& handler) noexcept;

    [[nodiscard]] std::expected<void,UserConfigFailure> require_handler(std::string_view setting) const;
    [[nodiscard]] std::expected<void,UserConfigFailure> validate_snapshot(
        const elysia::bootstrap::UserConfigData& settings) const;
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> apply_snapshot(
        const elysia::bootstrap::UserConfigData& settings,
        bool continue_after_failure = false);
    void restore_restart_required(bool restart_required) noexcept;
    elysia::bootstrap::UserConfigData _current_settings;
    elysia::bootstrap::UserConfigData _persisted_snapshot;
    IUserConfigChangeHandler* _change_handler = nullptr;
    bool _vsync_restart_pending = false;
};
}
