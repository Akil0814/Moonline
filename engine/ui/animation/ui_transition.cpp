#include "ui_transition.h"

#include <algorithm>
#include <cmath>

void UiTransition::set_hidden_state(const UiTransitionState& state)
{
    _hidden_state = state;
}

const UiTransitionState& UiTransition::hidden_state() const
{
    return _hidden_state;
}

void UiTransition::set_shown_state(const UiTransitionState& state)
{
    _shown_state = state;
}

const UiTransitionState& UiTransition::shown_state() const
{
    return _shown_state;
}

void UiTransition::set_duration_seconds(double duration_seconds)
{
    _duration_seconds = duration_seconds > 0.0 ? duration_seconds : 0.0;
}

double UiTransition::duration_seconds() const
{
    return _duration_seconds;
}

void UiTransition::set_easing(UiEasing easing)
{
    _easing = easing;
}

UiEasing UiTransition::easing() const
{
    return _easing;
}

void UiTransition::play_forward()
{
    _is_forward = true;
    _elapsed_seconds = 0.0;
    _is_playing = _duration_seconds > 0.0;
    _current_state = _is_playing ? _hidden_state : _shown_state;
}

void UiTransition::play_backward()
{
    _is_forward = false;
    _elapsed_seconds = 0.0;
    _is_playing = _duration_seconds > 0.0;
    _current_state = _is_playing ? _shown_state : _hidden_state;
}

void UiTransition::jump_to_hidden()
{
    _is_playing = false;
    _is_forward = false;
    _elapsed_seconds = 0.0;
    _current_state = _hidden_state;
}

void UiTransition::jump_to_shown()
{
    _is_playing = false;
    _is_forward = true;
    _elapsed_seconds = 0.0;
    _current_state = _shown_state;
}

void UiTransition::update(double delta_seconds)
{
    if (!_is_playing)
    {
        return;
    }

    _elapsed_seconds += std::max(0.0, delta_seconds);
    const double duration = _duration_seconds > 0.0 ? _duration_seconds : 1.0;
    const float raw_t = static_cast<float>(std::clamp(_elapsed_seconds / duration, 0.0, 1.0));
    const float eased_t = apply_easing(_easing, raw_t);

    if (_is_forward)
    {
        _current_state = interpolate(_hidden_state, _shown_state, eased_t);
    }
    else
    {
        _current_state = interpolate(_shown_state, _hidden_state, eased_t);
    }

    if (raw_t >= 1.0f)
    {
        _is_playing = false;
        _current_state = _is_forward ? _shown_state : _hidden_state;
    }
}

const UiTransitionState& UiTransition::current_state() const
{
    return _current_state;
}

bool UiTransition::is_playing() const
{
    return _is_playing;
}

bool UiTransition::is_shown() const
{
    return !_is_playing && _current_state._background_alpha == _shown_state._background_alpha
        && _current_state._transform.translation.nearly_equals(_shown_state._transform.translation)
        && _current_state._transform.scale.nearly_equals(_shown_state._transform.scale);
}

bool UiTransition::is_hidden() const
{
    return !_is_playing && _current_state._background_alpha == _hidden_state._background_alpha
        && _current_state._transform.translation.nearly_equals(_hidden_state._transform.translation)
        && _current_state._transform.scale.nearly_equals(_hidden_state._transform.scale);
}

bool UiTransition::is_forward() const
{
    return _is_forward;
}

float UiTransition::apply_easing(UiEasing easing, float t)
{
    switch (easing)
    {
    case UiEasing::Linear:
        return t;

    case UiEasing::EaseInQuad:
        return t * t;

    case UiEasing::EaseOutQuad:
        return 1.0f - (1.0f - t) * (1.0f - t);

    case UiEasing::EaseInOutQuad:
        if (t < 0.5f)
        {
            return 2.0f * t * t;
        }

        return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;

    case UiEasing::EaseOutBack:
    {
        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
    }
    }

    return t;
}

UiTransitionState UiTransition::interpolate(
    const UiTransitionState& from,
    const UiTransitionState& to,
    float t
)
{
    UiTransitionState result;
    result._transform.translation = {
        from._transform.translation.x + (to._transform.translation.x - from._transform.translation.x) * t,
        from._transform.translation.y + (to._transform.translation.y - from._transform.translation.y) * t
    };
    result._transform.scale = {
        from._transform.scale.x + (to._transform.scale.x - from._transform.scale.x) * t,
        from._transform.scale.y + (to._transform.scale.y - from._transform.scale.y) * t
    };

    const float alpha = static_cast<float>(from._background_alpha)
        + (static_cast<float>(to._background_alpha) - static_cast<float>(from._background_alpha)) * t;
    result._background_alpha = static_cast<Uint8>(std::clamp(alpha, 0.0f, 255.0f));
    return result;
}
