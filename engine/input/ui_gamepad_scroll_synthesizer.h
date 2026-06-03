#pragma once

#include "input_types.h"

#include <SDL.h>
#include <optional>

class UiGamepadScrollSynthesizer
{
public:
    void process_event(const SDL_Event& event);
    void on_context_changed(InputContext context);
    std::optional<InputEvent> synthesize(
        InputContext context,
        InputDevice current_device,
        bool device_switched_this_frame
    );
    void reset();

private:
    float normalize_axis(float axis_value) const;

private:
    float _left_stick_x = 0.0f;
    float _left_stick_y = 0.0f;
    float _scroll_accumulator = 0.0f;
};
