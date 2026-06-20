#pragma once

#include "gameplay_input_state.h"

namespace arcneco::input
{
struct GameplayInputFrame
{
    GameplayInputState state;
    elysia::input::InputDevice active_device = elysia::input::InputDevice::Unknown;
    bool device_switched_this_frame = false;
};
}
