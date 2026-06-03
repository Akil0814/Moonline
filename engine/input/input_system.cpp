#include "input_system.h"

void InputSystem::begin_frame()
{
    _state.begin_frame();
    _events.clear();
    _device_tracker.begin_frame();
}

void InputSystem::end_frame()
{
    append_synthesized_event();
}

void InputSystem::process_event(const SDL_Event& event)
{
    _ui_gamepad_scroll_synthesizer.process_event(event);

    const InputDeviceUpdateResult device_update = _device_tracker.process_event(event);
    if (device_update.should_clear_state)
    {
        _state.clear();
    }

    if (device_update.should_reset_gamepad_state)
    {
        _ui_gamepad_scroll_synthesizer.reset();
    }

    if (!device_update.should_translate)
    {
        return;
    }

    translate_event(event);
}

InputSnapshot InputSystem::snapshot() const
{
    return {
        _state,
        _context,
        _device_tracker.current_device(),
        _device_tracker.device_switched_this_frame()
    };
}

const std::vector<InputEvent>& InputSystem::events() const
{
    return _events;
}

InputDevice InputSystem::current_device() const
{
    return _device_tracker.current_device();
}

void InputSystem::set_context(InputContext context)
{
    _context = context;
    _ui_gamepad_scroll_synthesizer.on_context_changed(_context);
}

InputContext InputSystem::context() const
{
    return _context;
}

void InputSystem::translate_event(const SDL_Event& event)
{
    if (event.type == SDL_QUIT)
    {
        append_event({ InputAction::Exit, InputEventType::Pressed, InputDevice::Unknown });
        return;
    }

    const InputTranslator* translator = select_translator(_device_tracker.current_device());
    if (!translator)
    {
        return;
    }

    std::vector<InputEvent> input_events = translator->translate_event(event);

    for (const InputEvent& input_event : input_events)
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

void InputSystem::append_synthesized_event()
{
    const auto synthesized_event = _ui_gamepad_scroll_synthesizer.synthesize(
        _context,
        _device_tracker.current_device(),
        _device_tracker.device_switched_this_frame()
    );
    if (synthesized_event)
    {
        append_event(*synthesized_event);
    }
}
