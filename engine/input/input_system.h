#pragma once

#include "raw_input_frame.h"
#include "input_device_tracker.h"

#include "translator/gamepad_input_translator.h"
#include "translator/keyboard_mouse_input_translator.h"
#include "translator/input_translator.h"

#include <SDL.h>
#include <vector>

class InputSystem
{
public:
    void begin_frame();
    void end_frame();
    void process_event(const SDL_Event& event);
    RawInputFrame frame() const;
    const std::vector<RawInputEvent>& events() const;
    InputDevice current_device() const;

private:
    void translate_event(const SDL_Event& event);
    const InputTranslator* select_translator(InputDevice device) const;
    void apply_event(const RawInputEvent& event);
    void append_event(const RawInputEvent& event);
    bool should_clear_state_for_event(const SDL_Event& event) const;

private:
    RawInputState _state;
    std::vector<RawInputEvent> _events;
    InputDeviceTracker _device_tracker;
    KeyboardMouseInputTranslator _keyboard_mouse_translator;
    GamepadInputTranslator _gamepad_translator;
};
