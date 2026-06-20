#pragma once

#include "input_types.h"

namespace elysia::input
{
class InputState;

struct InputSnapshot
{
    const InputState& state;
    InputContext context = InputContext::Gameplay;
    InputDevice device = InputDevice::Unknown;
    bool device_switched_this_frame = false;
};

}
