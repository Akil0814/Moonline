#pragma once

#include "../../engine/input/raw_input_types.h"

enum class GameplayAction
{
    None = 0,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Jump,
    Attack,
    Special,
    Guard,
    Dash,
    Pause,
    Count
};

enum class GameplayInputEventType
{
    None = 0,
    ActionPressed,
    ActionReleased,
    AxisChanged
};

struct GameplayInputEvent
{
    GameplayAction action = GameplayAction::None;
    GameplayInputEventType type = GameplayInputEventType::None;
    InputDevice device = InputDevice::Unknown;
    RawInputAxis axis = RawInputAxis::None;
    float axis_value = 0.0f;
};
