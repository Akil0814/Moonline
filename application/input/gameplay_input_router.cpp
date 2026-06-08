#include "gameplay_input_router.h"

namespace
{
struct GameplayBinding
{
    GameplayAction action;
    RawInputControl control;
};

constexpr GameplayBinding k_gameplay_bindings[] = {
    { GameplayAction::MoveLeft, RawInputControl::KeyA },
    { GameplayAction::MoveLeft, RawInputControl::KeyLeft },
    { GameplayAction::MoveRight, RawInputControl::KeyD },
    { GameplayAction::MoveRight, RawInputControl::KeyRight },
    { GameplayAction::MoveUp, RawInputControl::KeyW },
    { GameplayAction::MoveUp, RawInputControl::KeyUp },
    { GameplayAction::MoveDown, RawInputControl::KeyS },
    { GameplayAction::MoveDown, RawInputControl::KeyDown },
    { GameplayAction::Jump, RawInputControl::KeySpace },
    { GameplayAction::Attack, RawInputControl::KeyJ },
    { GameplayAction::Special, RawInputControl::KeyK },
    { GameplayAction::Guard, RawInputControl::KeyL },
    { GameplayAction::Dash, RawInputControl::KeyLeftShift },
    { GameplayAction::Dash, RawInputControl::KeyRightShift },
    { GameplayAction::Pause, RawInputControl::KeyP },
    { GameplayAction::Attack, RawInputControl::MouseLeft },
    { GameplayAction::Guard, RawInputControl::MouseRight },
    { GameplayAction::MoveLeft, RawInputControl::GamepadDPadLeft },
    { GameplayAction::MoveRight, RawInputControl::GamepadDPadRight },
    { GameplayAction::MoveUp, RawInputControl::GamepadDPadUp },
    { GameplayAction::MoveDown, RawInputControl::GamepadDPadDown },
    { GameplayAction::Jump, RawInputControl::GamepadSouth },
    { GameplayAction::Attack, RawInputControl::GamepadWest },
    { GameplayAction::Special, RawInputControl::GamepadNorth },
    { GameplayAction::Guard, RawInputControl::GamepadLeftShoulder },
    { GameplayAction::Dash, RawInputControl::GamepadRightShoulder },
    { GameplayAction::Pause, RawInputControl::GamepadStart }
};
}

GameplayInputFrame GameplayInputRouter::route_frame(const RawInputFrame& raw_input) const
{
    GameplayInputFrame gameplay_input;
    gameplay_input.active_device = raw_input.active_device;
    gameplay_input.device_switched_this_frame = raw_input.device_switched_this_frame;

    for (int action_index = static_cast<int>(GameplayAction::None) + 1;
        action_index < static_cast<int>(GameplayAction::Count);
        ++action_index)
    {
        const GameplayAction action = static_cast<GameplayAction>(action_index);
        gameplay_input.state.set(
            action,
            is_action_pressed(raw_input.state, action),
            is_action_just_pressed(raw_input.state, action),
            is_action_just_released(raw_input.state, action)
        );
    }

    return gameplay_input;
}

std::vector<GameplayInputEvent> GameplayInputRouter::route_event(const RawInputEvent& raw_event) const
{
    std::vector<GameplayInputEvent> events;

    switch (raw_event.type)
    {
    case RawInputEventType::ControlPressed:
    case RawInputEventType::ControlReleased:
    {
        const GameplayAction action = action_from_control(raw_event.control);
        if (action == GameplayAction::None)
        {
            return events;
        }

        GameplayInputEvent event;
        event.action = action;
        event.type = raw_event.type == RawInputEventType::ControlPressed
            ? GameplayInputEventType::ActionPressed
            : GameplayInputEventType::ActionReleased;
        event.device = raw_event.device;
        events.push_back(event);
        break;
    }

    case RawInputEventType::AxisChanged:
    {
        GameplayInputEvent event;
        event.type = GameplayInputEventType::AxisChanged;
        event.device = raw_event.device;
        event.axis = raw_event.axis;
        event.axis_value = raw_event.axis_value;
        events.push_back(event);
        break;
    }

    case RawInputEventType::MouseWheel:
    case RawInputEventType::TextInput:
    case RawInputEventType::TextEditing:
    case RawInputEventType::None:
    default:
        break;
    }

    return events;
}

bool GameplayInputRouter::is_action_pressed(const RawInputState& state, GameplayAction action) const
{
    for (const GameplayBinding& binding : k_gameplay_bindings)
    {
        if (binding.action == action && state.is_pressed(binding.control))
        {
            return true;
        }
    }

    return false;
}

bool GameplayInputRouter::is_action_just_pressed(const RawInputState& state, GameplayAction action) const
{
    for (const GameplayBinding& binding : k_gameplay_bindings)
    {
        if (binding.action == action && state.is_just_pressed(binding.control))
        {
            return true;
        }
    }

    return false;
}

bool GameplayInputRouter::is_action_just_released(const RawInputState& state, GameplayAction action) const
{
    for (const GameplayBinding& binding : k_gameplay_bindings)
    {
        if (binding.action == action && state.is_just_released(binding.control))
        {
            return true;
        }
    }

    return false;
}

GameplayAction GameplayInputRouter::action_from_control(RawInputControl control) const
{
    for (const GameplayBinding& binding : k_gameplay_bindings)
    {
        if (binding.control == control)
        {
            return binding.action;
        }
    }

    return GameplayAction::None;
}
