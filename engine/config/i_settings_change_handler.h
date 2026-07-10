#pragma once

#include "user_settings_failure.h"

#include <expected>
#include <string_view>

namespace elysia::config
{
class ISettingsChangeHandler
{
public:
    virtual ~ISettingsChangeHandler() = default;

    virtual std::expected<void,UserSettingsFailure> apply_master_volume(int value) = 0;
    virtual std::expected<void,UserSettingsFailure> apply_music_volume(int value) = 0;
    virtual std::expected<void,UserSettingsFailure> apply_sound_volume(int value) = 0;
    virtual std::expected<void,UserSettingsFailure> apply_language(std::string_view language) = 0;
    virtual std::expected<void,UserSettingsFailure> apply_target_fps(double value) = 0;
    virtual std::expected<void,UserSettingsFailure> apply_window_size(int width,int height) = 0;
    virtual std::expected<void,UserSettingsFailure> apply_fullscreen(bool value) = 0;
};
}
