#include "gameplay_input_router.h"

namespace
{
struct GameplayBinding
{
    arcneco::input::GameplayAction action;
    elysia::input::RawInputControl control;
};

constexpr GameplayBinding k_gameplay_bindings[] = {
    { arcneco::input::GameplayAction::MoveLeft, elysia::input::RawInputControl::KeyA },
    { arcneco::input::GameplayAction::MoveLeft, elysia::input::RawInputControl::KeyLeft },
    { arcneco::input::GameplayAction::MoveRight, elysia::input::RawInputControl::KeyD },
    { arcneco::input::GameplayAction::MoveRight, elysia::input::RawInputControl::KeyRight },
    { arcneco::input::GameplayAction::MoveUp, elysia::input::RawInputControl::KeyW },
    { arcneco::input::GameplayAction::MoveUp, elysia::input::RawInputControl::KeyUp },
    { arcneco::input::GameplayAction::MoveDown, elysia::input::RawInputControl::KeyS },
    { arcneco::input::GameplayAction::MoveDown, elysia::input::RawInputControl::KeyDown },
    { arcneco::input::GameplayAction::Jump, elysia::input::RawInputControl::KeySpace },
    { arcneco::input::GameplayAction::Attack, elysia::input::RawInputControl::KeyJ },
    { arcneco::input::GameplayAction::Special, elysia::input::RawInputControl::KeyK },
    { arcneco::input::GameplayAction::Guard, elysia::input::RawInputControl::KeyL },
    { arcneco::input::GameplayAction::Dash, elysia::input::RawInputControl::KeyLeftShift },
    { arcneco::input::GameplayAction::Dash, elysia::input::RawInputControl::KeyRightShift },
    { arcneco::input::GameplayAction::Pause, elysia::input::RawInputControl::KeyP },
    { arcneco::input::GameplayAction::Attack, elysia::input::RawInputControl::MouseLeft },
    { arcneco::input::GameplayAction::Guard, elysia::input::RawInputControl::MouseRight },
    { arcneco::input::GameplayAction::MoveLeft, elysia::input::RawInputControl::GamepadDPadLeft },
    { arcneco::input::GameplayAction::MoveRight, elysia::input::RawInputControl::GamepadDPadRight },
    { arcneco::input::GameplayAction::MoveUp, elysia::input::RawInputControl::GamepadDPadUp },
    { arcneco::input::GameplayAction::MoveDown, elysia::input::RawInputControl::GamepadDPadDown },
    { arcneco::input::GameplayAction::Jump, elysia::input::RawInputControl::GamepadSouth },
    { arcneco::input::GameplayAction::Attack, elysia::input::RawInputControl::GamepadWest },
    { arcneco::input::GameplayAction::Special, elysia::input::RawInputControl::GamepadNorth },
    { arcneco::input::GameplayAction::Guard, elysia::input::RawInputControl::GamepadLeftShoulder },
    { arcneco::input::GameplayAction::Dash, elysia::input::RawInputControl::GamepadRightShoulder },
    { arcneco::input::GameplayAction::Pause, elysia::input::RawInputControl::GamepadStart }
};
}

arcneco::input::GameplayInputFrame GameplayInputRouter::route_frame(const elysia::input::RawInputFrame& raw_input) const
{
    arcneco::input::GameplayInputFrame gameplay_input;
    gameplay_input.active_device = raw_input.active_device;
    gameplay_input.device_switched_this_frame = raw_input.device_switched_this_frame;

    for (int action_index = static_cast<int>(arcneco::input::GameplayAction::None) + 1;
        action_index < static_cast<int>(arcneco::input::GameplayAction::Count);
        ++action_index)
    {
        const arcneco::input::GameplayAction action = static_cast<arcneco::input::GameplayAction>(action_index);
        gameplay_input.state.set(
            action,
            is_action_pressed(raw_input.state, action),
            is_action_just_pressed(raw_input.state, action),
            is_action_just_released(raw_input.state, action)
        );
    }

    return gameplay_input;
}

std::vector<arcneco::input::GameplayInputEvent> GameplayInputRouter::route_event(const elysia::input::RawInputEvent& raw_event) const
{
    std::vector<arcneco::input::GameplayInputEvent> events;

    switch (raw_event.type)
    {
    case elysia::input::RawInputEventType::ControlPressed:
    case elysia::input::RawInputEventType::ControlReleased:
    {
        const arcneco::input::GameplayAction action = action_from_control(raw_event.control);
        if (action == arcneco::input::GameplayAction::None)
        {
            return events;
        }

        arcneco::input::GameplayInputEvent event;
        event.action = action;
        event.type = raw_event.type == elysia::input::RawInputEventType::ControlPressed
            ? arcneco::input::GameplayInputEventType::ActionPressed
            : arcneco::input::GameplayInputEventType::ActionReleased;
        event.device = raw_event.device;
        events.push_back(event);
        break;
    }

    case elysia::input::RawInputEventType::AxisChanged:
    {
        arcneco::input::GameplayInputEvent event;
        event.type = arcneco::input::GameplayInputEventType::AxisChanged;
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

bool GameplayInputRouter::is_action_pressed(const elysia::input::RawInputState& state, arcneco::input::GameplayAction action) const
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

bool GameplayInputRouter::is_action_just_pressed(const elysia::input::RawInputState& state, arcneco::input::GameplayAction action) const
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

bool GameplayInputRouter::is_action_just_released(const elysia::input::RawInputState& state, arcneco::input::GameplayAction action) const
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

arcneco::input::GameplayAction GameplayInputRouter::action_from_control(elysia::input::RawInputControl control) const
{
    for (const GameplayBinding& binding : k_gameplay_bindings)
    {
        if (binding.control == control)
        {
            return binding.action;
        }
    }

    return arcneco::input::GameplayAction::None;
}
