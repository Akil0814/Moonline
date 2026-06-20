#include "gameplay_input_router.h"

namespace
{
struct GameplayBinding
{
    GameplayAction action;
    elysia::input::RawInputControl control;
};

constexpr GameplayBinding k_gameplay_bindings[] = {
    { GameplayAction::MoveLeft, elysia::input::RawInputControl::KeyA },
    { GameplayAction::MoveLeft, elysia::input::RawInputControl::KeyLeft },
    { GameplayAction::MoveRight, elysia::input::RawInputControl::KeyD },
    { GameplayAction::MoveRight, elysia::input::RawInputControl::KeyRight },
    { GameplayAction::MoveUp, elysia::input::RawInputControl::KeyW },
    { GameplayAction::MoveUp, elysia::input::RawInputControl::KeyUp },
    { GameplayAction::MoveDown, elysia::input::RawInputControl::KeyS },
    { GameplayAction::MoveDown, elysia::input::RawInputControl::KeyDown },
    { GameplayAction::Jump, elysia::input::RawInputControl::KeySpace },
    { GameplayAction::Attack, elysia::input::RawInputControl::KeyJ },
    { GameplayAction::Special, elysia::input::RawInputControl::KeyK },
    { GameplayAction::Guard, elysia::input::RawInputControl::KeyL },
    { GameplayAction::Dash, elysia::input::RawInputControl::KeyLeftShift },
    { GameplayAction::Dash, elysia::input::RawInputControl::KeyRightShift },
    { GameplayAction::Pause, elysia::input::RawInputControl::KeyP },
    { GameplayAction::Attack, elysia::input::RawInputControl::MouseLeft },
    { GameplayAction::Guard, elysia::input::RawInputControl::MouseRight },
    { GameplayAction::MoveLeft, elysia::input::RawInputControl::GamepadDPadLeft },
    { GameplayAction::MoveRight, elysia::input::RawInputControl::GamepadDPadRight },
    { GameplayAction::MoveUp, elysia::input::RawInputControl::GamepadDPadUp },
    { GameplayAction::MoveDown, elysia::input::RawInputControl::GamepadDPadDown },
    { GameplayAction::Jump, elysia::input::RawInputControl::GamepadSouth },
    { GameplayAction::Attack, elysia::input::RawInputControl::GamepadWest },
    { GameplayAction::Special, elysia::input::RawInputControl::GamepadNorth },
    { GameplayAction::Guard, elysia::input::RawInputControl::GamepadLeftShoulder },
    { GameplayAction::Dash, elysia::input::RawInputControl::GamepadRightShoulder },
    { GameplayAction::Pause, elysia::input::RawInputControl::GamepadStart }
};
}

GameplayInputFrame GameplayInputRouter::route_frame(const elysia::input::RawInputFrame& raw_input) const
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

std::vector<GameplayInputEvent> GameplayInputRouter::route_event(const elysia::input::RawInputEvent& raw_event) const
{
    std::vector<GameplayInputEvent> events;

    switch (raw_event.type)
    {
    case elysia::input::RawInputEventType::ControlPressed:
    case elysia::input::RawInputEventType::ControlReleased:
    {
        const GameplayAction action = action_from_control(raw_event.control);
        if (action == GameplayAction::None)
        {
            return events;
        }

        GameplayInputEvent event;
        event.action = action;
        event.type = raw_event.type == elysia::input::RawInputEventType::ControlPressed
            ? GameplayInputEventType::ActionPressed
            : GameplayInputEventType::ActionReleased;
        event.device = raw_event.device;
        events.push_back(event);
        break;
    }

    case elysia::input::RawInputEventType::AxisChanged:
    {
        GameplayInputEvent event;
        event.type = GameplayInputEventType::AxisChanged;
        event.device = raw_event.device;
        event.axis = raw_event.axis;
        event.axis_value = raw_event.axis_value;
        events.push_back(event);
        break;
    }

    case elysia::input::RawInputEventType::MouseWheel:
    case elysia::input::RawInputEventType::TextInput:
    case elysia::input::RawInputEventType::TextEditing:
    case elysia::input::RawInputEventType::None:
    default:
        break;
    }

    return events;
}

bool GameplayInputRouter::is_action_pressed(const elysia::input::RawInputState& state, GameplayAction action) const
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

bool GameplayInputRouter::is_action_just_pressed(const elysia::input::RawInputState& state, GameplayAction action) const
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

bool GameplayInputRouter::is_action_just_released(const elysia::input::RawInputState& state, GameplayAction action) const
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

GameplayAction GameplayInputRouter::action_from_control(elysia::input::RawInputControl control) const
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
