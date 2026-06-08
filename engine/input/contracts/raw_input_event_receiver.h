#pragma once

#include "../raw_input_types.h"

class RawInputEventReceiver
{
public:
    virtual ~RawInputEventReceiver() = default;
    virtual bool on_raw_input_event(const RawInputEvent& event) = 0;
};
