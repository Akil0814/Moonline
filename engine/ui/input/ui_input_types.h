#pragma once

#include "../../input/raw_input_types.h"

#include <string>

namespace elysia::ui
{
enum class UiAction
{
    None = 0,
    NavigateLeft,
    NavigateRight,
    NavigateUp,
    NavigateDown,
    Confirm,
    Cancel,
    Tab,
    Backspace,
    DeleteKey,
    Home,
    End,
    Count
};

enum class UiInputEventType
{
    None = 0,
    ActionPressed,
    ActionReleased,
    MouseMoved,
    PointerPressed,
    PointerReleased,
    MouseWheel,
    TextInput,
    TextEditing,
    AxisChanged
};

struct UiInputEvent
{
    UiAction action = UiAction::None;
    UiInputEventType type = UiInputEventType::None;
    elysia::input::InputDevice device = elysia::input::InputDevice::Unknown;
    elysia::input::RawInputControl control = elysia::input::RawInputControl::None;
    elysia::input::RawInputAxis axis = elysia::input::RawInputAxis::None;
    int mouse_x = 0;
    int mouse_y = 0;
    int wheel_x = 0;
    int wheel_y = 0;
    float axis_value = 0.0f;
    std::string text;
};

}
