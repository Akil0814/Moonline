#pragma once

#include "../gameplay_input_types.h"

namespace arcneco::input
{
class GameplayInputEventReceiver
{
public:
    virtual ~GameplayInputEventReceiver() = default;
    virtual bool on_gameplay_input_event(const GameplayInputEvent& event) = 0;
};
}
