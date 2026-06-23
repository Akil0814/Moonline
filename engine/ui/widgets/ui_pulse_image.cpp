#include "ui_pulse_image.h"

#include <cmath>

namespace elysia::ui
{
namespace
{
constexpr double k_pi = 3.14159265358979323846;
}

UiPulseImage::UiPulseImage(SDL_Texture* texture, elysia::core::Vector2 pos, elysia::core::Vector2 size, int order)
    : UiImage(texture, pos, size, order)
{
    _timer.set_one_shot(true);
}

UiPulseImage::UiPulseImage(SDL_Texture* texture, elysia::core::Rect rect, int order)
    : UiImage(texture, rect, order)
{
    _timer.set_one_shot(true);
}

UiPulseImage::UiPulseImage(
    SDL_Texture* texture,
    elysia::core::Vector2 center,
    elysia::core::Vector2 image_size,
    UiImageCenterTag tag,
    int order
)
    : UiImage(texture, center, image_size, tag, order)
{
    _timer.set_one_shot(true);
}

UiPulseImage::UiPulseImage(
    SDL_Texture* texture,
    elysia::core::Vector2 center,
    elysia::core::Vector2 source_size,
    elysia::core::Vector2 render_size,
    UiImageCenterTag tag,
    int order
)
    : UiImage(texture, center, source_size, render_size, tag, order)
{
    _timer.set_one_shot(true);
}

void UiPulseImage::reset() noexcept
{
    UiImage::reset();

    _state = PulseState::Idle;
    _elapsed = 0.0;
    _completed_cycles = 0;

    set_opacity(_max_alpha);
    _timer.restart();
    _timer.pause();
}

void UiPulseImage::configure_playback(
    UiPulseImageMode mode,
    double hold_time,
    double pulse_in_duration,
    double pulse_out_duration,
    int pulse_cycles,
    std::uint8_t min_alpha,
    std::uint8_t max_alpha
)
{
    _mode = mode;
    _hold_time = hold_time > 0.0 ? hold_time : 0.0;
    _pulse_in_duration = pulse_in_duration > 0.0 ? pulse_in_duration : 0.0;
    _pulse_out_duration = pulse_out_duration > 0.0 ? pulse_out_duration : 0.0;
    _pulse_cycles = pulse_cycles;

    if (min_alpha <= max_alpha)
    {
        _min_alpha = min_alpha;
        _max_alpha = max_alpha;
    }
    else
    {
        _min_alpha = max_alpha;
        _max_alpha = min_alpha;
    }

    _timer.set_wait_time(_hold_time);
    _timer.set_on_timeout([this] {
        switch (_state)
        {
        case PulseState::HoldingMax:
            _elapsed = 0.0;
            _state = PulseState::PulsingDown;
            break;
        case PulseState::HoldingMin:
            _elapsed = 0.0;
            _state = PulseState::PulsingUp;
            break;
        default:
            break;
        }
    });
}

void UiPulseImage::play()
{
    _elapsed = 0.0;
    _completed_cycles = 0;

    if (started_from_min())
    {
        set_opacity(_min_alpha);
        _state = PulseState::PulsingUp;
    }
    else
    {
        set_opacity(_max_alpha);
        _state = PulseState::PulsingDown;
    }

    if (_pulse_cycles == 0)
    {
        finish();
    }
}

void UiPulseImage::update(double delta)
{
    if (_state == PulseState::Idle || _state == PulseState::Finished)
        return;

    switch (_state)
    {
    case PulseState::PulsingUp:
        update_pulse_up(delta);
        break;
    case PulseState::HoldingMax:
    case PulseState::HoldingMin:
        _timer.update(delta);
        break;
    case PulseState::PulsingDown:
        update_pulse_down(delta);
        break;
    default:
        break;
    }

    if (_state == PulseState::Finished && _on_end)
        _on_end();
}

void UiPulseImage::set_on_end(PulseImageOnEnd on_end)
{
    _on_end = on_end;
}

void UiPulseImage::update_pulse_up(double delta)
{
    _elapsed += delta;

    const double t = ease_in_out(ratio(_elapsed, _pulse_in_duration));
    set_opacity(lerp_opacity(_min_alpha, _max_alpha, t));

    if (t < 1.0)
        return;

    _elapsed = 0.0;
    set_opacity(_max_alpha);

    if (complete_cycle_if_needed(true))
    {
        finish();
        return;
    }

    start_hold(true);
}

void UiPulseImage::update_pulse_down(double delta)
{
    _elapsed += delta;

    const double t = ease_in_out(ratio(_elapsed, _pulse_out_duration));
    set_opacity(lerp_opacity(_max_alpha, _min_alpha, t));

    if (t < 1.0)
        return;

    _elapsed = 0.0;
    set_opacity(_min_alpha);

    if (complete_cycle_if_needed(false))
    {
        finish();
        return;
    }

    start_hold(false);
}

void UiPulseImage::start_hold(bool at_max)
{
    if (_hold_time <= 0.0)
    {
        _state = at_max ? PulseState::PulsingDown : PulseState::PulsingUp;
        return;
    }

    _state = at_max ? PulseState::HoldingMax : PulseState::HoldingMin;
    _timer.restart();
}

void UiPulseImage::finish()
{
    _state = PulseState::Finished;
    _timer.pause();
}

bool UiPulseImage::started_from_min() const
{
    return _mode == UiPulseImageMode::MinToMax;
}

bool UiPulseImage::complete_cycle_if_needed(bool reached_max)
{
    const bool cycle_completed =
        (started_from_min() && !reached_max) ||
        (!started_from_min() && reached_max);

    if (!cycle_completed || _pulse_cycles < 0)
        return false;

    ++_completed_cycles;
    return _completed_cycles >= _pulse_cycles;
}

double UiPulseImage::ratio(double value, double max_value) const
{
    if (max_value <= 0.0)
        return 1.0;

    double t = value / max_value;
    if (t < 0.0)
        return 0.0;
    if (t > 1.0)
        return 1.0;
    return t;
}

double UiPulseImage::ease_in_out(double t) const
{
    return 0.5 - 0.5 * std::cos(k_pi * t);
}

std::uint8_t UiPulseImage::lerp_opacity(std::uint8_t from, std::uint8_t to, double t) const
{
    const double alpha = static_cast<double>(from) + (static_cast<double>(to) - static_cast<double>(from)) * t;
    return static_cast<std::uint8_t>(alpha);
}

}
