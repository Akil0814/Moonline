#pragma once

#include "../ui_input_frame.h"

namespace elysia::ui
{
class UiInputFrameReceiver
{
public:
    virtual ~UiInputFrameReceiver() = default;
    virtual void on_ui_input_frame(const UiInputFrame& input) = 0;
};

}
