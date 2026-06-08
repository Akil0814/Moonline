#include "input_system.h"

#include <cmath>

void InputSystem::begin_frame()
{
    _state.begin_frame();
    _events.clear();
    _device_tracker.begin_frame();
    _mouse_delta_x = 0;
    _mouse_delta_y = 0;
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
        _gamepad_translator.reset();
        return;
    }

    const InputDeviceUpdateResult device_update = _device_tracker.process_event(event);
    if (device_update.should_clear_state)
    {
        _state.clear();
    }

    if (device_update.should_reset_gamepad_state)
    {
        _gamepad_translator.reset();
    }

    if (!device_update.should_translate)
    {
        return;
    }

    translate_event(event, device_update.event_device);
}

RawInputFrame InputSystem::frame() const
{
    return {
        _state,
        _device_tracker.current_device(),
        _device_tracker.device_switched_this_frame(),
        _mouse_x,
        _mouse_y,
        _mouse_delta_x,
        _mouse_delta_y
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

void InputSystem::set_renderer(SDL_Renderer* renderer)
{
    _renderer = renderer;
}

void InputSystem::translate_event(const SDL_Event& event, InputDevice event_device)
{
    InputTranslator* translator = select_translator(event_device);
    if (!translator)
    {
        return;
    }

    std::vector<RawInputEvent> input_events = translator->translate_event(event);

    for (const RawInputEvent& input_event : input_events)
    {
        append_event(convert_mouse_event_to_logical(input_event));
    }
}

InputTranslator* InputSystem::select_translator(InputDevice device)
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

RawInputEvent InputSystem::convert_mouse_event_to_logical(const RawInputEvent& event) const
{
    if (event.device != InputDevice::Mouse)
    {
        return event;
    }

    RawInputEvent converted_event = event;
    const bool has_position = event.type == RawInputEventType::MouseMoved
        || event.mouse_button != 0;
    if (!has_position)
    {
        return converted_event;
    }

    convert_window_to_logical(
        event.mouse_x,
        event.mouse_y,
        converted_event.mouse_x,
        converted_event.mouse_y
    );

    if (event.type == RawInputEventType::MouseMoved)
    {
        if (_has_mouse_position)
        {
            converted_event.mouse_delta_x = converted_event.mouse_x - _mouse_x;
            converted_event.mouse_delta_y = converted_event.mouse_y - _mouse_y;
        }
        else
        {
            converted_event.mouse_delta_x = 0;
            converted_event.mouse_delta_y = 0;
        }
    }
    else
    {
        converted_event.mouse_delta_x = 0;
        converted_event.mouse_delta_y = 0;
    }

    return converted_event;
}

void InputSystem::update_mouse_frame_cache(const RawInputEvent& event)
{
    if (event.device != InputDevice::Mouse)
    {
        return;
    }

    const bool has_position = event.type == RawInputEventType::MouseMoved
        || event.mouse_button != 0;
    if (!has_position)
    {
        return;
    }

    _mouse_x = event.mouse_x;
    _mouse_y = event.mouse_y;

    if (event.type == RawInputEventType::MouseMoved)
    {
        _mouse_delta_x += event.mouse_delta_x;
        _mouse_delta_y += event.mouse_delta_y;
    }

    _has_mouse_position = true;
}

void InputSystem::convert_window_to_logical(int window_x, int window_y, int& logical_x, int& logical_y) const
{
    if (!_renderer)
    {
        logical_x = window_x;
        logical_y = window_y;
        return;
    }

    float converted_x = static_cast<float>(window_x);
    float converted_y = static_cast<float>(window_y);
    SDL_RenderWindowToLogical(_renderer, window_x, window_y, &converted_x, &converted_y);

    logical_x = static_cast<int>(std::lround(converted_x));
    logical_y = static_cast<int>(std::lround(converted_y));
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
    update_mouse_frame_cache(event);
    _events.push_back(event);
}

bool InputSystem::should_clear_state_for_event(const SDL_Event& event) const
{
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST;
}
