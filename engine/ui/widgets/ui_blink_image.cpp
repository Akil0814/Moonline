#include "ui_blink_image.h"

namespace elysia::ui
{
UiBlinkImage::UiBlinkImage(SDL_Texture* texture, elysia::core::Vector2 pos, elysia::core::Vector2 size, int order)
    : UiImage(texture, pos, size, order)
{
    _timer.set_one_shot(true);
}

UiBlinkImage::UiBlinkImage(SDL_Texture* texture, elysia::core::Rect rect, int order)
    : UiImage(texture, rect, order)
{
    _timer.set_one_shot(true);
}

UiBlinkImage::UiBlinkImage(
    SDL_Texture* texture,
    elysia::core::Vector2 center,
    elysia::core::Vector2 image_size,
    UiFromCenterTag,
    int order
)
    : UiImage(texture, center, image_size, from_center, order)
{
    _timer.set_one_shot(true);
}

UiBlinkImage::UiBlinkImage(
    SDL_Texture* texture,
    elysia::core::Vector2 center,
    elysia::core::Vector2 source_size,
    elysia::core::Vector2 render_size,
    UiFromCenterTag,
    int order
)
    : UiImage(texture, center, source_size, render_size, from_center, order)
{
    _timer.set_one_shot(true);
}

void UiBlinkImage::reset() noexcept
{
    UiImage::reset();

    _is_playing = false;
    _hold_pending = false;
    _completed_cycles = 0;
    _current_visible = true;

    set_opacity(255);
    _timer.restart();
    _timer.pause();
}

void UiBlinkImage::configure_playback(
    UiBlinkImageMode mode,
    double hold_time,
    double visible_duration,
    double hidden_duration,
    int blink_cycles
)
{
    _mode = mode;
    _hold_time = hold_time > 0.0 ? hold_time : 0.0;
    _visible_duration = visible_duration > 0.0 ? visible_duration : 0.0;
    _hidden_duration = hidden_duration > 0.0 ? hidden_duration : 0.0;
    _blink_cycles = blink_cycles;

    _timer.set_on_timeout([this] {
        advance_phase();
    });
}

void UiBlinkImage::play()
{
    _is_playing = true;
    _completed_cycles = 0;
    _hold_pending = _hold_time > 0.0;
    _current_visible = _mode == UiBlinkImageMode::VisibleFirst;

    apply_current_phase();

    if (_blink_cycles == 0)
    {
        finish();
        return;
    }

    if (_hold_pending)
    {
        _timer.set_wait_time(_hold_time);
        _timer.restart();
        return;
    }

    if (_blink_cycles < 0 && _visible_duration <= 0.0 && _hidden_duration <= 0.0)
    {
        finish();
        return;
    }

    schedule_current_phase();
}

void UiBlinkImage::update(double delta)
{
    if (!_is_playing)
        return;

    _timer.update(delta);
}

void UiBlinkImage::set_on_end(BlinkImageOnEnd on_end)
{
    _on_end = on_end;
}

void UiBlinkImage::advance_phase()
{
    if (!_is_playing)
        return;

    if (_hold_pending)
    {
        _hold_pending = false;
        _current_visible = !_current_visible;
        apply_current_phase();
        schedule_current_phase();
        return;
    }

    const bool start_visible = _mode == UiBlinkImageMode::VisibleFirst;
    const bool next_visible = !_current_visible;
    const bool returning_to_start = next_visible == start_visible;

    _current_visible = next_visible;
    apply_current_phase();

    if (returning_to_start && _blink_cycles >= 0)
    {
        ++_completed_cycles;
        if (_completed_cycles >= _blink_cycles)
        {
            finish();
            return;
        }
    }

    schedule_current_phase();
}

void UiBlinkImage::schedule_current_phase()
{
    const double duration = _current_visible ? _visible_duration : _hidden_duration;

    if (duration <= 0.0)
    {
        advance_phase();
        return;
    }

    _timer.set_wait_time(duration);
    _timer.restart();
}

void UiBlinkImage::finish()
{
    _is_playing = false;
    _hold_pending = false;
    _timer.pause();

    if (_on_end)
        _on_end();
}

void UiBlinkImage::apply_current_phase()
{
    set_opacity(_current_visible ? 255 : 0);
}

}