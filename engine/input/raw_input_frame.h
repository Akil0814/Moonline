#pragma once

#include "raw_input_state.h"

struct RawInputFrame
{
    RawInputState state;
    InputDevice active_device = InputDevice::Unknown;
    bool device_switched_this_frame = false;
    int mouse_x = 0;
    int mouse_y = 0;
    int mouse_delta_x = 0;
    int mouse_delta_y = 0;
};
