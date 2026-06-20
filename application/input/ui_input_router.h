#pragma once

#include "../../engine/input/raw_input_frame.h"
#include "../../engine/ui/input/ui_gamepad_scroll_synthesizer.h"
#include "../../engine/ui/input/ui_input_frame.h"

#include <vector>

class UiInputRouter
{
public:
    elysia::ui::UiInputFrame route_frame(const elysia::input::RawInputFrame& raw_input) const;
    std::vector<elysia::ui::UiInputEvent> route_event(const elysia::input::RawInputEvent& raw_event) const;
    std::vector<elysia::ui::UiInputEvent> synthesize_events(const elysia::input::RawInputFrame& raw_input);
    void reset_transient_state();

private:
    [[nodiscard]] bool is_action_pressed(const elysia::input::RawInputState& state, elysia::ui::UiAction action) const;
    [[nodiscard]] bool is_action_just_pressed(const elysia::input::RawInputState& state, elysia::ui::UiAction action) const;
    [[nodiscard]] bool is_action_just_released(const elysia::input::RawInputState& state, elysia::ui::UiAction action) const;
    [[nodiscard]] elysia::ui::UiAction action_from_control(elysia::input::RawInputControl control) const;

private:
    elysia::ui::UiGamepadScrollSynthesizer _gamepad_scroll_synthesizer;
};
