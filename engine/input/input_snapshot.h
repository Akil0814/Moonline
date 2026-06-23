#pragma once

#include "input_types.h"

namespace elysia::input
{
class InputState;

// Legacy snapshot path kept for older UI/layout code that is not on the current raw-input scene chain.
struct InputSnapshot
{
    const InputState& state;
    InputContext context = InputContext::Gameplay;
    InputDevice device = InputDevice::Unknown;
    bool device_switched_this_frame = false;
};

}
