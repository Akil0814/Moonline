#include "../../input/input_system.h"

class InputEventReceiver
{
public:
    virtual ~InputEventReceiver() = default;
    virtual bool on_input_event(const InputEvent& event) = 0;
};