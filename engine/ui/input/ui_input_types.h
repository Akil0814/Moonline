#pragma once

#include "../../input/raw_input_types.h"

#include <string>

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
    MouseWheel,
    TextInput,
    TextEditing,
    AxisChanged
};

struct UiInputEvent
{
    UiAction action = UiAction::None;
    UiInputEventType type = UiInputEventType::None;
    InputDevice device = InputDevice::Unknown;
    RawInputAxis axis = RawInputAxis::None;
    int wheel_x = 0;
    int wheel_y = 0;
    float axis_value = 0.0f;
    std::string text;
};
