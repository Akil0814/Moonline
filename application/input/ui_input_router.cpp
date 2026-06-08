#include "ui_input_router.h"

namespace
{
struct UiBinding
{
    UiAction action;
    RawInputControl control;
};

constexpr UiBinding k_ui_bindings[] = {
    { UiAction::NavigateLeft, RawInputControl::KeyA },
    { UiAction::NavigateLeft, RawInputControl::KeyLeft },
    { UiAction::NavigateRight, RawInputControl::KeyD },
    { UiAction::NavigateRight, RawInputControl::KeyRight },
    { UiAction::NavigateUp, RawInputControl::KeyW },
    { UiAction::NavigateUp, RawInputControl::KeyUp },
    { UiAction::NavigateDown, RawInputControl::KeyS },
    { UiAction::NavigateDown, RawInputControl::KeyDown },
    { UiAction::Confirm, RawInputControl::KeyEnter },
    { UiAction::Confirm, RawInputControl::KeyNumpadEnter },
    { UiAction::Confirm, RawInputControl::GamepadSouth },
    { UiAction::Cancel, RawInputControl::KeyEscape },
    { UiAction::Cancel, RawInputControl::GamepadEast },
    { UiAction::Tab, RawInputControl::KeyTab },
    { UiAction::Backspace, RawInputControl::KeyBackspace },
    { UiAction::DeleteKey, RawInputControl::KeyDelete },
    { UiAction::Home, RawInputControl::KeyHome },
    { UiAction::End, RawInputControl::KeyEnd },
    { UiAction::NavigateLeft, RawInputControl::GamepadDPadLeft },
    { UiAction::NavigateRight, RawInputControl::GamepadDPadRight },
    { UiAction::NavigateUp, RawInputControl::GamepadDPadUp },
    { UiAction::NavigateDown, RawInputControl::GamepadDPadDown }
};
}

UiInputFrame UiInputRouter::route_frame(const RawInputFrame& raw_input) const
{
    UiInputFrame ui_input;
    ui_input.active_device = raw_input.active_device;
    ui_input.device_switched_this_frame = raw_input.device_switched_this_frame;

    for (int action_index = static_cast<int>(UiAction::None) + 1;
        action_index < static_cast<int>(UiAction::Count);
        ++action_index)
    {
        const UiAction action = static_cast<UiAction>(action_index);
        ui_input.state.set(
            action,
            is_action_pressed(raw_input.state, action),
            is_action_just_pressed(raw_input.state, action),
            is_action_just_released(raw_input.state, action)
        );
    }

    return ui_input;
}

std::vector<UiInputEvent> UiInputRouter::route_event(const RawInputEvent& raw_event) const
{
    std::vector<UiInputEvent> events;

    switch (raw_event.type)
    {
    case RawInputEventType::ControlPressed:
    case RawInputEventType::ControlReleased:
    {
        const UiAction action = action_from_control(raw_event.control);
        if (action == UiAction::None)
        {
            return events;
        }

        UiInputEvent event;
        event.action = action;
        event.type = raw_event.type == RawInputEventType::ControlPressed
            ? UiInputEventType::ActionPressed
            : UiInputEventType::ActionReleased;
        event.device = raw_event.device;
        events.push_back(event);
        break;
    }

    case RawInputEventType::MouseWheel:
    {
        UiInputEvent event;
        event.type = UiInputEventType::MouseWheel;
        event.device = raw_event.device;
        event.wheel_x = raw_event.wheel_x;
        event.wheel_y = raw_event.wheel_y;
        events.push_back(event);
        break;
    }

    case RawInputEventType::TextInput:
    {
        UiInputEvent event;
        event.type = UiInputEventType::TextInput;
        event.device = raw_event.device;
        event.text = raw_event.text;
        events.push_back(event);
        break;
    }

    case RawInputEventType::TextEditing:
    {
        UiInputEvent event;
        event.type = UiInputEventType::TextEditing;
        event.device = raw_event.device;
        event.text = raw_event.text;
        events.push_back(event);
        break;
    }

    case RawInputEventType::AxisChanged:
    {
        UiInputEvent event;
        event.type = UiInputEventType::AxisChanged;
        event.device = raw_event.device;
        event.axis = raw_event.axis;
        event.axis_value = raw_event.axis_value;
        events.push_back(event);
        break;
    }

    case RawInputEventType::None:
    default:
        break;
    }

    return events;
}

std::vector<UiInputEvent> UiInputRouter::synthesize_events(const RawInputFrame& raw_input)
{
    std::vector<UiInputEvent> events;

    const auto synthesized_event = _gamepad_scroll_synthesizer.synthesize(raw_input);
    if (synthesized_event)
    {
        events.push_back(*synthesized_event);
    }

    return events;
}

void UiInputRouter::reset_transient_state()
{
    _gamepad_scroll_synthesizer.reset();
}

bool UiInputRouter::is_action_pressed(const RawInputState& state, UiAction action) const
{
    for (const UiBinding& binding : k_ui_bindings)
    {
        if (binding.action == action && state.is_pressed(binding.control))
        {
            return true;
        }
    }

    return false;
}

bool UiInputRouter::is_action_just_pressed(const RawInputState& state, UiAction action) const
{
    for (const UiBinding& binding : k_ui_bindings)
    {
        if (binding.action == action && state.is_just_pressed(binding.control))
        {
            return true;
        }
    }

    return false;
}

bool UiInputRouter::is_action_just_released(const RawInputState& state, UiAction action) const
{
    for (const UiBinding& binding : k_ui_bindings)
    {
        if (binding.action == action && state.is_just_released(binding.control))
        {
            return true;
        }
    }

    return false;
}

UiAction UiInputRouter::action_from_control(RawInputControl control) const
{
    for (const UiBinding& binding : k_ui_bindings)
    {
        if (binding.control == control)
        {
            return binding.action;
        }
    }

    return UiAction::None;
}
