#include "ui_gamepad_scroll_synthesizer.h"

#include <algorithm>
#include <cmath>

void UiGamepadScrollSynthesizer::process_event(const SDL_Event& event)
{
    if (event.type != SDL_CONTROLLERAXISMOTION)
    {
        return;
    }

    const float axis_value = std::clamp(
        static_cast<float>(event.caxis.value) / 32767.0f,
        -1.0f,
        1.0f
    );

    switch (event.caxis.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTX:
        _left_stick_x = axis_value;
        break;

    case SDL_CONTROLLER_AXIS_LEFTY:
        _left_stick_y = axis_value;
        break;

    default:
        break;
    }
}

void UiGamepadScrollSynthesizer::on_context_changed(InputContext context)
{
    if (context != InputContext::UI)
    {
        _scroll_accumulator = 0.0f;
    }
}

std::optional<InputEvent> UiGamepadScrollSynthesizer::synthesize(
    InputContext context,
    InputDevice current_device,
    bool device_switched_this_frame
)
{
    if (context != InputContext::UI)
    {
        return std::nullopt;
    }

    if (current_device != InputDevice::Gamepad || device_switched_this_frame)
    {
        return std::nullopt;
    }

    const float normalized_x = normalize_axis(_left_stick_x);
    const float normalized_y = normalize_axis(_left_stick_y);
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

    InputEvent scroll_event;
    scroll_event.type = InputEventType::MouseWheel;
    scroll_event.device = InputDevice::Gamepad;
    scroll_event.wheel_y = wheel_steps;
    return scroll_event;
}

void UiGamepadScrollSynthesizer::reset()
{
    _left_stick_x = 0.0f;
    _left_stick_y = 0.0f;
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
