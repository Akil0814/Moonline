#pragma once

#include "ui_input_state.h"

namespace elysia::ui
{
struct UiInputFrame
{
    UiInputState state;
    elysia::input::InputDevice active_device = elysia::input::InputDevice::Unknown;
    bool device_switched_this_frame = false;
};

}
