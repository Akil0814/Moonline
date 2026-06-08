#pragma once

#include "../input/contracts/ui_input_event_receiver.h"

class UiFocusable : public UiInputEventReceiver
{
public:
    virtual ~UiFocusable() = default;

    virtual void set_focused(bool focused) = 0;
    [[nodiscard]] virtual bool is_focused() const = 0;
};
