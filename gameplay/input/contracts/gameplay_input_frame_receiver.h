#pragma once

#include "../gameplay_input_frame.h"

namespace arcneco::input
{
class GameplayInputFrameReceiver
{
public:
    virtual ~GameplayInputFrameReceiver() = default;
    virtual void on_gameplay_input_frame(const GameplayInputFrame& input) = 0;
};
}
