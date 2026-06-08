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
    void set_renderer(SDL_Renderer* renderer);

private:
    void translate_event(const SDL_Event& event, InputDevice event_device);
    InputTranslator* select_translator(InputDevice device);
    RawInputEvent convert_mouse_event_to_logical(const RawInputEvent& event) const;
    void update_mouse_frame_cache(const RawInputEvent& event);
    void convert_window_to_logical(int window_x, int window_y, int& logical_x, int& logical_y) const;
    void apply_event(const RawInputEvent& event);
    void append_event(const RawInputEvent& event);
    bool should_clear_state_for_event(const SDL_Event& event) const;

private:
    RawInputState _state;
    std::vector<RawInputEvent> _events;
    InputDeviceTracker _device_tracker;
    SDL_Renderer* _renderer = nullptr;
    KeyboardMouseInputTranslator _keyboard_mouse_translator;
    GamepadInputTranslator _gamepad_translator;
    int _mouse_x = 0;
    int _mouse_y = 0;
    int _mouse_delta_x = 0;
    int _mouse_delta_y = 0;
    bool _has_mouse_position = false;
};
