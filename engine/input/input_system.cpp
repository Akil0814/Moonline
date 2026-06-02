#include "input_system.h"
#include "input_translator.h"

#include <algorithm>
#include <cmath>

void InputSystem::begin_frame()
{
    _state.begin_frame();
    _events.clear();
    _device_switched_this_frame = false;
}

void InputSystem::end_frame()
{
    emit_ui_gamepad_scroll();
}

void InputSystem::process_event(const SDL_Event& event)
{
    update_controller_axis_state(event);

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
            _gamepad_scroll_accumulator = 0.0f;
            return;
        }
        else if (is_keyboard_or_mouse(event_device))
        {
            if (_current_device == InputDevice::Gamepad)
            {
                _state.clear();
                reset_gamepad_scroll_state();
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
    if (_context != InputContext::UI)
    {
        _gamepad_scroll_accumulator = 0.0f;
    }
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
        append_event(input_event);
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

void InputSystem::append_event(const InputEvent& event)
{
    apply_event(event);
    _events.push_back(event);
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

void InputSystem::update_controller_axis_state(const SDL_Event& event)
{
    if (event.type != SDL_CONTROLLERAXISMOTION)
    {
        return;
    }

    const float axis_value = std::clamp(
        static_cast<float>(event.caxis.value) / 32767.0f,
        -1.0f,
        1.0f
    );

    switch (event.caxis.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTX:
        _left_stick_x = axis_value;
        break;

    case SDL_CONTROLLER_AXIS_LEFTY:
        _left_stick_y = axis_value;
        break;

    default:
        break;
    }
}

void InputSystem::emit_ui_gamepad_scroll()
{
    if (_context != InputContext::UI)
    {
        return;
    }

    if (_current_device != InputDevice::Gamepad || _device_switched_this_frame)
    {
        return;
    }

    const auto normalize_axis = [](float axis_value) -> float
    {
        const float deadzone = 0.22f;
        const float abs_value = std::fabs(axis_value);
        if (abs_value <= deadzone)
        {
            return 0.0f;
        }

        const float normalized = (abs_value - deadzone) / (1.0f - deadzone);
        return axis_value < 0.0f ? -normalized : normalized;
    };

    const float normalized_x = normalize_axis(_left_stick_x);
    const float normalized_y = normalize_axis(_left_stick_y);
    if (normalized_y == 0.0f)
    {
        _gamepad_scroll_accumulator = 0.0f;
        return;
    }

    const float vertical_weight = std::max(0.0f, 1.0f - std::fabs(normalized_x));
    const float scroll_strength = (-normalized_y) * vertical_weight * 0.75f;
    _gamepad_scroll_accumulator += scroll_strength;

    int wheel_steps = 0;
    if (_gamepad_scroll_accumulator >= 1.0f)
    {
        wheel_steps = static_cast<int>(std::floor(_gamepad_scroll_accumulator));
    }
    else if (_gamepad_scroll_accumulator <= -1.0f)
    {
        wheel_steps = static_cast<int>(std::ceil(_gamepad_scroll_accumulator));
    }

    if (wheel_steps == 0)
    {
        return;
    }

    wheel_steps = std::clamp(wheel_steps, -3, 3);
    _gamepad_scroll_accumulator -= static_cast<float>(wheel_steps);

    InputEvent scroll_event;
    scroll_event.type = InputEventType::MouseWheel;
    scroll_event.device = InputDevice::Gamepad;
    scroll_event.wheel_y = wheel_steps;
    append_event(scroll_event);
}

void InputSystem::reset_gamepad_scroll_state()
{
    _left_stick_x = 0.0f;
    _left_stick_y = 0.0f;
    _gamepad_scroll_accumulator = 0.0f;
}
