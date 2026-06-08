#include "ui_gamepad_scroll_synthesizer.h"

#include <algorithm>
#include <cmath>

std::optional<UiInputEvent> UiGamepadScrollSynthesizer::synthesize(const RawInputFrame& input)
{
    if (input.active_device != InputDevice::Gamepad || input.device_switched_this_frame)
    {
        _scroll_accumulator = 0.0f;
        return std::nullopt;
    }

    const float normalized_x = normalize_axis(
        input.state.axis_value(RawInputAxis::GamepadLeftX)
    );
    const float normalized_y = normalize_axis(
        input.state.axis_value(RawInputAxis::GamepadLeftY)
    );

    if (normalized_y == 0.0f)
    {
        _scroll_accumulator = 0.0f;
        return std::nullopt;
    }

    const float vertical_weight = std::max(0.0f, 1.0f - std::fabs(normalized_x));
    const float scroll_strength = (-normalized_y) * vertical_weight * 0.75f;
    _scroll_accumulator += scroll_strength;

    int wheel_steps = 0;
    if (_scroll_accumulator >= 1.0f)
    {
        wheel_steps = static_cast<int>(std::floor(_scroll_accumulator));
    }
    else if (_scroll_accumulator <= -1.0f)
    {
        wheel_steps = static_cast<int>(std::ceil(_scroll_accumulator));
    }

    if (wheel_steps == 0)
    {
        return std::nullopt;
    }

    wheel_steps = std::clamp(wheel_steps, -3, 3);
    _scroll_accumulator -= static_cast<float>(wheel_steps);

    UiInputEvent scroll_event;
    scroll_event.type = UiInputEventType::MouseWheel;
    scroll_event.device = InputDevice::Gamepad;
    scroll_event.wheel_y = wheel_steps;
    return scroll_event;
}

void UiGamepadScrollSynthesizer::reset()
{
    _scroll_accumulator = 0.0f;
}

float UiGamepadScrollSynthesizer::normalize_axis(float axis_value) const
{
    const float deadzone = 0.22f;
    const float abs_value = std::fabs(axis_value);
    if (abs_value <= deadzone)
    {
        return 0.0f;
    }

    const float normalized = (abs_value - deadzone) / (1.0f - deadzone);
    return axis_value < 0.0f ? -normalized : normalized;
}
