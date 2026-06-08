#pragma once

#include "../../engine/input/raw_input_frame.h"
#include "../../gameplay/input/gameplay_input_frame.h"

#include <vector>

class GameplayInputRouter
{
public:
    GameplayInputFrame route_frame(const RawInputFrame& raw_input) const;
    std::vector<GameplayInputEvent> route_event(const RawInputEvent& raw_event) const;

private:
    [[nodiscard]] bool is_action_pressed(const RawInputState& state, GameplayAction action) const;
    [[nodiscard]] bool is_action_just_pressed(const RawInputState& state, GameplayAction action) const;
    [[nodiscard]] bool is_action_just_released(const RawInputState& state, GameplayAction action) const;
    [[nodiscard]] GameplayAction action_from_control(RawInputControl control) const;
};
