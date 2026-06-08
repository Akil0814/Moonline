#pragma once

#include "ui_input_state.h"

struct UiInputFrame
{
    UiInputState state;
    InputDevice active_device = InputDevice::Unknown;
    bool device_switched_this_frame = false;
};
