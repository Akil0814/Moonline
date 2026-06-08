#include "input_translator.h"

RawInputEventType InputTranslator::input_event_type(bool pressed) const
{
    return pressed ? RawInputEventType::ControlPressed : RawInputEventType::ControlReleased;
}

void InputTranslator::append_event(
    std::vector<RawInputEvent>& events,
    RawInputControl control,
    RawInputEventType type,
    InputDevice device
) const
{
    events.push_back({ control, RawInputAxis::None, type, device });
}
