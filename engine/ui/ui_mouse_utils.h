#pragma once

#include "../input/input_system.h"

#include <SDL.h>

inline SDL_Point ui_logical_mouse_position_from_window_position(int window_x, int window_y)
{
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

inline SDL_Point ui_logical_mouse_position()
{
    int window_x = 0;
    int window_y = 0;
    SDL_GetMouseState(&window_x, &window_y);
    return ui_logical_mouse_position_from_window_position(window_x, window_y);
}

struct UiMouseState
{
    SDL_Point _position{ 0, 0 };
    bool _left_button_down = false;
};

inline UiMouseState ui_current_mouse_state()
{
    int window_x = 0;
    int window_y = 0;
    const Uint32 mouse_buttons = SDL_GetMouseState(&window_x, &window_y);
    return {
        ui_logical_mouse_position_from_window_position(window_x, window_y),
        (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0
    };
}

inline bool ui_left_mouse_pressed(const UiMouseState& mouse_state, bool was_mouse_down)
{
    return mouse_state._left_button_down && !was_mouse_down;
}

inline bool ui_left_mouse_released(const UiMouseState& mouse_state, bool was_mouse_down)
{
    return !mouse_state._left_button_down && was_mouse_down;
}

inline bool ui_contains_point(const SDL_Rect& rect, const SDL_Point& point)
{
    return SDL_PointInRect(&point, &rect) == SDL_TRUE;
}

inline bool ui_contains_point(const SDL_Rect& rect, int x, int y)
{
    return ui_contains_point(rect, SDL_Point{ x, y });
}

template <typename RectGetter>
int ui_find_hit_index(size_t count, const SDL_Point& point, RectGetter&& rect_getter)
{
    for (size_t index = 0; index < count; ++index)
    {
        const SDL_Rect* rect = rect_getter(index);
        if (rect && ui_contains_point(*rect, point))
        {
            return static_cast<int>(index);
        }
    }

    return -1;
}

template <typename RectGetter>
int ui_find_hit_index_reverse(size_t count, const SDL_Point& point, RectGetter&& rect_getter)
{
    for (size_t offset = count; offset > 0; --offset)
    {
        const size_t index = offset - 1;
        const SDL_Rect* rect = rect_getter(index);
        if (rect && ui_contains_point(*rect, point))
        {
            return static_cast<int>(index);
        }
    }

    return -1;
}

inline bool ui_left_mouse_pressed_in_rect(
    const UiMouseState& mouse_state,
    bool was_mouse_down,
    const SDL_Rect& rect
)
{
    return ui_left_mouse_pressed(mouse_state, was_mouse_down)
        && ui_contains_point(rect, mouse_state._position);
}

inline bool ui_left_mouse_released_in_rect(
    const UiMouseState& mouse_state,
    bool was_mouse_down,
    const SDL_Rect& rect
)
{
    return ui_left_mouse_released(mouse_state, was_mouse_down)
        && ui_contains_point(rect, mouse_state._position);
}

inline void ui_reset_mouse_press_state(bool& was_mouse_down)
{
    was_mouse_down = false;
}

inline void ui_sync_mouse_press_state(bool& was_mouse_down, const UiMouseState& mouse_state)
{
    was_mouse_down = mouse_state._left_button_down;
}

inline void ui_reset_mouse_drag_state(bool& is_dragging, bool& was_mouse_down)
{
    is_dragging = false;
    was_mouse_down = false;
}

inline bool ui_begin_mouse_drag(
    bool& is_dragging,
    const UiMouseState& mouse_state,
    bool was_mouse_down,
    const SDL_Rect& drag_start_rect
)
{
    if (!ui_left_mouse_pressed_in_rect(mouse_state, was_mouse_down, drag_start_rect))
    {
        return false;
    }

    is_dragging = true;
    return true;
}

inline bool ui_drag_in_progress(const UiMouseState& mouse_state, bool is_dragging)
{
    return mouse_state._left_button_down && is_dragging;
}

inline void ui_sync_mouse_drag_state(bool& is_dragging, bool& was_mouse_down, const UiMouseState& mouse_state)
{
    if (!mouse_state._left_button_down)
    {
        is_dragging = false;
    }

    ui_sync_mouse_press_state(was_mouse_down, mouse_state);
}

inline bool ui_mouse_input_allowed(const InputSnapshot& input)
{
    return input.device != InputDevice::Gamepad && !input.device_switched_this_frame;
}
