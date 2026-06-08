#pragma once

#include <string>

enum class InputDevice
{
    Keyboard,
    Mouse,
    Gamepad,
    Unknown
};

enum class RawInputControl
{
    None = 0,

    KeyA,
    KeyD,
    KeyW,
    KeyS,
    KeyLeft,
    KeyRight,
    KeyUp,
    KeyDown,
    KeyEnter,
    KeyNumpadEnter,
    KeyEscape,
    KeyP,
    KeySpace,
    KeyJ,
    KeyK,
    KeyL,
    KeyLeftShift,
    KeyRightShift,
    KeyBackspace,
    KeyDelete,
    KeyHome,
    KeyEnd,
    KeyTab,

    MouseLeft,
    MouseRight,

    GamepadSouth,
    GamepadEast,
    GamepadWest,
    GamepadNorth,
    GamepadLeftShoulder,
    GamepadRightShoulder,
    GamepadStart,
    GamepadDPadLeft,
    GamepadDPadRight,
    GamepadDPadUp,
    GamepadDPadDown,

    Count
};

enum class RawInputAxis
{
    None = 0,
    GamepadLeftX,
    GamepadLeftY,
    Count
};

enum class RawInputEventType
{
    None = 0,
    ControlPressed,
    ControlReleased,
    MouseWheel,
    TextInput,
    TextEditing,
    AxisChanged
};

struct RawInputEvent
{
    RawInputControl control = RawInputControl::None;
    RawInputAxis axis = RawInputAxis::None;
    RawInputEventType type = RawInputEventType::None;
    InputDevice device = InputDevice::Unknown;
    int wheel_x = 0;
    int wheel_y = 0;
    float axis_value = 0.0f;
    std::string text;
};
