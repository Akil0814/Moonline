#pragma once

#include "../input/input_system.h"

#include <SDL.h>

inline SDL_Point ui_logical_mouse_position()
{
    int window_x = 0;
    int window_y = 0;
    SDL_GetMouseState(&window_x, &window_y);

    SDL_Window* mouse_window = SDL_GetMouseFocus();
    if (!mouse_window)
    {
        return { window_x, window_y };
    }

    SDL_Renderer* renderer = SDL_GetRenderer(mouse_window);
    if (!renderer)
    {
        return { window_x, window_y };
    }

    int logical_width = 0;
    int logical_height = 0;
    SDL_RenderGetLogicalSize(renderer, &logical_width, &logical_height);
    if (logical_width <= 0 || logical_height <= 0)
    {
        return { window_x, window_y };
    }

#if SDL_VERSION_ATLEAST(2, 0, 18)
    float logical_x = static_cast<float>(window_x);
    float logical_y = static_cast<float>(window_y);
    SDL_RenderWindowToLogical(renderer, window_x, window_y, &logical_x, &logical_y);
    return { static_cast<int>(logical_x), static_cast<int>(logical_y) };
#else
    int window_width = 0;
    int window_height = 0;
    SDL_GetWindowSize(mouse_window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
    {
        return { window_x, window_y };
    }

    const float scale_x = static_cast<float>(window_width) / static_cast<float>(logical_width);
    const float scale_y = static_cast<float>(window_height) / static_cast<float>(logical_height);
    const float scale = (scale_x < scale_y) ? scale_x : scale_y;

    if (scale <= 0.0f)
    {
        return { window_x, window_y };
    }

    const float viewport_width = static_cast<float>(logical_width) * scale;
    const float viewport_height = static_cast<float>(logical_height) * scale;
    const float viewport_x = (static_cast<float>(window_width) - viewport_width) * 0.5f;
    const float viewport_y = (static_cast<float>(window_height) - viewport_height) * 0.5f;

    const float logical_x = (static_cast<float>(window_x) - viewport_x) / scale;
    const float logical_y = (static_cast<float>(window_y) - viewport_y) / scale;

    return { static_cast<int>(logical_x), static_cast<int>(logical_y) };
#endif
}

inline bool ui_mouse_input_allowed(const InputSnapshot& input)
{
    return input.device != InputDevice::Gamepad && !input.device_switched_this_frame;
}
