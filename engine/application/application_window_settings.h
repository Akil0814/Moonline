#pragma once

#include "../bootstrap/runtime_settings.h"

#include <SDL.h>

#include <expected>
#include <functional>
#include <string>

namespace elysia::application::detail
{
struct ApplicationWindowOperations
{
    std::function<int(Uint32)> set_fullscreen;
    std::function<void(int,int)> set_size;
    std::function<void()> center;
    std::function<std::string()> error_message;
};

[[nodiscard]] std::expected<void,std::string> apply_window_settings(
    const elysia::bootstrap::WindowSettings& settings,
    const ApplicationWindowOperations& operations);
}
