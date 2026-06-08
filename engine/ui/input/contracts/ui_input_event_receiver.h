#pragma once

#include "../ui_input_types.h"

class UiInputEventReceiver
{
public:
    virtual ~UiInputEventReceiver() = default;
    virtual bool on_ui_input_event(const UiInputEvent& event) = 0;
};
