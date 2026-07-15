#pragma once

#include "user_config_failure.h"

#include <expected>
#include <string_view>

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
}
