#include "user_config.h"

namespace elysia::config
{
namespace
{
[[nodiscard]] std::unexpected<UserConfigFailure> invalid(std::string setting,std::string message)
{
    return std::unexpected(UserConfigFailure{UserConfigError::InvalidValue,std::move(setting),std::move(message)});
}

[[nodiscard]] bool is_volume(int value) noexcept { return value >= 0 && value <= 100; }
}

int UserConfig::window_width() const noexcept { return _current_settings.window_width; }
int UserConfig::window_height() const noexcept { return _current_settings.window_height; }
bool UserConfig::fullscreen() const noexcept { return _current_settings.fullscreen; }
double UserConfig::target_fps() const noexcept { return _current_settings.target_fps; }
bool UserConfig::vsync() const noexcept { return _current_settings.vsync; }
int UserConfig::master_volume() const noexcept { return _current_settings.audio.master_volume; }
int UserConfig::music_volume() const noexcept { return _current_settings.audio.music_volume; }
int UserConfig::sound_volume() const noexcept { return _current_settings.audio.sound_volume; }
std::string_view UserConfig::language() const noexcept { return _current_settings.language; }

std::expected<void,UserConfigFailure> UserConfig::require_handler(std::string_view setting) const
{
    if (_change_handler) return {};
    return std::unexpected(UserConfigFailure{UserConfigError::ChangeHandlerUnavailable,std::string(setting),"A runtime settings change handler is not registered."});
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_window_size(int width,int height)
{
    if (width <= 0 || height <= 0) return invalid("window_size","Window width and height must be positive.");
    if (width == _current_settings.window_width && height == _current_settings.window_height) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("window_size"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_window_size(width,height); !applied) return std::unexpected(applied.error());
    _current_settings.window_width = width; _current_settings.window_height = height;
    return UserConfigApplyStatus::Applied;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_fullscreen(bool value)
{
    if (value == _current_settings.fullscreen) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("fullscreen"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_fullscreen(value); !applied) return std::unexpected(applied.error());
    _current_settings.fullscreen = value;
    return UserConfigApplyStatus::Applied;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_target_fps(double value)
{
    if (value <= 0.0) return invalid("target_fps","Target FPS must be positive.");
    if (value == _current_settings.target_fps) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("target_fps"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_target_fps(value); !applied) return std::unexpected(applied.error());
    _current_settings.target_fps = value;
    return UserConfigApplyStatus::Applied;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_vsync(bool value)
{
    if (value == _current_settings.vsync) return UserConfigApplyStatus::Applied;
    _current_settings.vsync = value;
    _vsync_restart_pending = true;
    return UserConfigApplyStatus::PendingRestart;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_master_volume(int value)
{
    if (!is_volume(value)) return invalid("master_volume","Volume must be within 0..100.");
    if (value == _current_settings.audio.master_volume) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("master_volume"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_master_volume(value); !applied) return std::unexpected(applied.error());
    _current_settings.audio.master_volume = value;
    return UserConfigApplyStatus::Applied;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_music_volume(int value)
{
    if (!is_volume(value)) return invalid("music_volume","Volume must be within 0..100.");
    if (value == _current_settings.audio.music_volume) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("music_volume"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_music_volume(value); !applied) return std::unexpected(applied.error());
    _current_settings.audio.music_volume = value;
    return UserConfigApplyStatus::Applied;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_sound_volume(int value)
{
    if (!is_volume(value)) return invalid("sound_volume","Volume must be within 0..100.");
    if (value == _current_settings.audio.sound_volume) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("sound_volume"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_sound_volume(value); !applied) return std::unexpected(applied.error());
    _current_settings.audio.sound_volume = value;
    return UserConfigApplyStatus::Applied;
}

std::expected<UserConfigApplyStatus,UserConfigFailure> UserConfig::set_language(std::string value)
{
    if (value.empty()) return invalid("language","Language must be non-empty.");
    if (value == _current_settings.language) return UserConfigApplyStatus::Applied;
    if (const auto handler = require_handler("language"); !handler) return std::unexpected(handler.error());
    if (const auto applied = _change_handler->apply_language(value); !applied) return std::unexpected(applied.error());
    _current_settings.language = std::move(value);
    return UserConfigApplyStatus::Applied;
}

elysia::bootstrap::UserConfigData UserConfig::snapshot() const { return _current_settings; }
bool UserConfig::is_dirty() const noexcept { return _current_settings != _persisted_snapshot; }
bool UserConfig::restart_required() const noexcept { return _vsync_restart_pending; }
void UserConfig::initialize(const elysia::bootstrap::UserConfigData& settings) noexcept { _current_settings = settings; _persisted_snapshot = settings; _vsync_restart_pending = false; }
void UserConfig::mark_persisted() noexcept { _persisted_snapshot = _current_settings; }
void UserConfig::reset() noexcept { _current_settings = {}; _persisted_snapshot = {}; _change_handler = nullptr; _vsync_restart_pending = false; }
void UserConfig::register_change_handler(IUserConfigChangeHandler& handler) noexcept { _change_handler = &handler; }
void UserConfig::unregister_change_handler(IUserConfigChangeHandler& handler) noexcept { if (_change_handler == &handler) _change_handler = nullptr; }
}
