#pragma once

#include "../../engine/input/raw_input_frame.h"
#include "../../engine/ui/input/ui_gamepad_scroll_synthesizer.h"
#include "../../engine/ui/input/ui_input_frame.h"

#include <vector>

class UiInputRouter
{
public:
    UiInputFrame route_frame(const RawInputFrame& raw_input) const;
    std::vector<UiInputEvent> route_event(const RawInputEvent& raw_event) const;
    std::vector<UiInputEvent> synthesize_events(const RawInputFrame& raw_input);
    void reset_transient_state();

private:
    [[nodiscard]] bool is_action_pressed(const RawInputState& state, UiAction action) const;
    [[nodiscard]] bool is_action_just_pressed(const RawInputState& state, UiAction action) const;
    [[nodiscard]] bool is_action_just_released(const RawInputState& state, UiAction action) const;
    [[nodiscard]] UiAction action_from_control(RawInputControl control) const;

private:
    UiGamepadScrollSynthesizer _gamepad_scroll_synthesizer;
};
