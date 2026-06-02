#include "input_system.h"
#include "input_translator.h"

void InputSystem::begin_frame()
{
    _state.begin_frame();
    _events.clear();
    _device_switched_this_frame = false;
}

void InputSystem::process_event(const SDL_Event& event)
{
    const InputDevice event_device = detect_event_device(event);
    if (event_device != InputDevice::Unknown)
    {
        if (_current_device == InputDevice::Unknown)
        {
            _current_device = event_device;
        }
        else if (event_device == InputDevice::Gamepad && _current_device != InputDevice::Gamepad)
        {
            _current_device = event_device;
            _device_switched_this_frame = true;
            _state.clear();
            return;
        }
        else if (is_keyboard_or_mouse(event_device))
        {
            if (_current_device == InputDevice::Gamepad)
            {
                _state.clear();
            }

            _current_device = event_device;
        }
    }

    translate_event(event);
}

InputSnapshot InputSystem::snapshot() const
{
    return { _state, _context, _current_device, _device_switched_this_frame };
}

const std::vector<InputEvent>& InputSystem::events() const
{
    return _events;
}

InputDevice InputSystem::current_device() const
{
    return _current_device;
}

void InputSystem::set_context(InputContext context)
{
    _context = context;
}

InputContext InputSystem::context() const
{
    return _context;
}

void InputSystem::translate_event(const SDL_Event& event)
{
    std::vector<InputEvent> input_events = InputTranslator::instance()->translate_event(event);

    for (const InputEvent& input_event : input_events)
    {
        apply_event(input_event);
        _events.push_back(input_event);
    }
}

void InputSystem::apply_event(const InputEvent& event)
{
    if (event.type != InputEventType::Pressed && event.type != InputEventType::Released)
    {
        return;
    }

    _state.set_pressed(event.action, event.type == InputEventType::Pressed);
}

InputDevice InputSystem::detect_event_device(const SDL_Event& event) const
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_TEXTEDITING:
    case SDL_TEXTINPUT:
        return InputDevice::Keyboard;

    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEWHEEL:
        return InputDevice::Mouse;

    case SDL_CONTROLLERAXISMOTION:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        return InputDevice::Gamepad;

    default:
        return InputDevice::Unknown;
    }
}

bool InputSystem::is_keyboard_or_mouse(InputDevice device) const
{
    return device == InputDevice::Keyboard || device == InputDevice::Mouse;
}
