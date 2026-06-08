#pragma once

#include "gameplay_input_state.h"

struct GameplayInputFrame
{
    GameplayInputState state;
    InputDevice active_device = InputDevice::Unknown;
    bool device_switched_this_frame = false;
};
