#pragma once

#include "raw_input_state.h"

struct RawInputFrame
{
    RawInputState state;
    InputDevice active_device = InputDevice::Unknown;
    bool device_switched_this_frame = false;
};
