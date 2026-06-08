#pragma once

#include "../raw_input_frame.h"

class RawInputFrameReceiver
{
public:
    virtual ~RawInputFrameReceiver() = default;
    virtual void on_raw_input_frame(const RawInputFrame& input) = 0;
};
