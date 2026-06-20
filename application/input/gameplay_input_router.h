#pragma once

#include "../../engine/input/raw_input_frame.h"
#include "../../gameplay/input/gameplay_input_frame.h"

#include <vector>

class GameplayInputRouter
{
public:
    arcneco::input::GameplayInputFrame route_frame(const elysia::input::RawInputFrame& raw_input) const;
    std::vector<arcneco::input::GameplayInputEvent> route_event(const elysia::input::RawInputEvent& raw_event) const;

private:
    [[nodiscard]] bool is_action_pressed(const elysia::input::RawInputState& state, arcneco::input::GameplayAction action) const;
    [[nodiscard]] bool is_action_just_pressed(const elysia::input::RawInputState& state, arcneco::input::GameplayAction action) const;
    [[nodiscard]] bool is_action_just_released(const elysia::input::RawInputState& state, arcneco::input::GameplayAction action) const;
    [[nodiscard]] arcneco::input::GameplayAction action_from_control(elysia::input::RawInputControl control) const;
};
