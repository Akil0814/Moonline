#pragma once

#include "../bootstrap/runtime_settings.h"
#include "i_user_config_change_handler.h"
#include "user_config_apply_status.h"

#include <expected>
#include <string>
#include <string_view>

namespace elysia::config
{
class UserConfig
{
public:
    [[nodiscard]] int window_width() const noexcept;
    [[nodiscard]] int window_height() const noexcept;
    [[nodiscard]] bool fullscreen() const noexcept;
    [[nodiscard]] double target_fps() const noexcept;
    [[nodiscard]] bool vsync() const noexcept;
    [[nodiscard]] int master_volume() const noexcept;
    [[nodiscard]] int music_volume() const noexcept;
    [[nodiscard]] int sound_volume() const noexcept;
    [[nodiscard]] std::string_view language() const noexcept;

    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_window_size(int width,int height);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_fullscreen(bool value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_target_fps(double value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_vsync(bool value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_master_volume(int value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_music_volume(int value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_sound_volume(int value);
    [[nodiscard]] std::expected<UserConfigApplyStatus,UserConfigFailure> set_language(std::string value);

    [[nodiscard]] elysia::bootstrap::UserConfigData snapshot() const;
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
    elysia::bootstrap::UserConfigData _current_settings;
    elysia::bootstrap::UserConfigData _persisted_snapshot;
    IUserConfigChangeHandler* _change_handler = nullptr;
    bool _vsync_restart_pending = false;
};
}
