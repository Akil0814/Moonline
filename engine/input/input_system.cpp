#include "input_system.h"

void InputSystem::begin_frame()
{
    _state.begin_frame();
    _events.clear();
    _device_tracker.begin_frame();
}

void InputSystem::end_frame()
{
}

void InputSystem::process_event(const SDL_Event& event)
{
    if (should_clear_state_for_event(event))
    {
        _state.clear();
        _device_tracker.reset();
        return;
    }

    const InputDeviceUpdateResult device_update = _device_tracker.process_event(event);
    if (device_update.should_clear_state)
    {
        _state.clear();
    }

    if (!device_update.should_translate)
    {
        return;
    }

    translate_event(event);
}

RawInputFrame InputSystem::frame() const
{
    return {
        _state,
        _device_tracker.current_device(),
        _device_tracker.device_switched_this_frame()
    };
}

const std::vector<RawInputEvent>& InputSystem::events() const
{
    return _events;
}

InputDevice InputSystem::current_device() const
{
    return _device_tracker.current_device();
}

void InputSystem::translate_event(const SDL_Event& event)
{
    const InputTranslator* translator = select_translator(_device_tracker.current_device());
    if (!translator)
    {
        return;
    }

    std::vector<RawInputEvent> input_events = translator->translate_event(event);

    for (const RawInputEvent& input_event : input_events)
    {
        append_event(input_event);
    }
}

const InputTranslator* InputSystem::select_translator(InputDevice device) const
{
    if (device == InputDevice::Gamepad)
    {
        return &_gamepad_translator;
    }

    if (device == InputDevice::Keyboard || device == InputDevice::Mouse)
    {
        return &_keyboard_mouse_translator;
    }

    return nullptr;
}

void InputSystem::apply_event(const RawInputEvent& event)
{
    if (event.type == RawInputEventType::ControlPressed)
    {
        _state.set_pressed(event.control, true);
        return;
    }

    if (event.type == RawInputEventType::ControlReleased)
    {
        _state.set_pressed(event.control, false);
        return;
    }

    if (event.type == RawInputEventType::AxisChanged)
    {
        _state.set_axis(event.axis, event.axis_value);
    }
}

void InputSystem::append_event(const RawInputEvent& event)
{
    apply_event(event);
    _events.push_back(event);
}

bool InputSystem::should_clear_state_for_event(const SDL_Event& event) const
{
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST;
}
