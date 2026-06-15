#include "ui_fade_image.h"
#include "../../core/render/render_command.h"

#include <SDL.h>

UiFadeImage::UiFadeImage(SDL_Texture* texture, Vector2 pos, Vector2 size,int order)
	:UiElement(pos,size,order), _texture(texture)
{
    _timer.set_one_shot(true);
    if (_texture)
        SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

UiFadeImage::UiFadeImage(SDL_Texture* texture, Rect rect, int order)
    :UiElement(rect, order), _texture(texture)
{
    _timer.set_one_shot(true);
    if (_texture)
        SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

void UiFadeImage::reset() noexcept
{
    UiElement::reset();
    _state = FadeState::Idle;
    _elapsed = 0.0;
    _alpha = 255;
    _timer.restart();
    _timer.pause();
}


void UiFadeImage::configure_playback(
    UiFadeImageMode mode,
    double hold_time,
    double fade_in_duration,
    double fade_out_duration
)
{
	_mode = mode;

    if (hold_time < 0)
        hold_time = 0.0;

    _hold_time = hold_time;
	_timer.set_wait_time(_hold_time);
    _timer.set_on_timeout([this] {
        switch (_mode)
        {
        case UiFadeImageMode::FadeIn:
            _state = FadeState::Finished;
            break;
        case UiFadeImageMode::FadeOut:
        case UiFadeImageMode::FadeInOut:
            _elapsed = 0.0;
            _state = FadeState::FadingOut;
                break;
        default:
            break;
        }
        });

    _fade_in_duration = fade_in_duration;
    _fade_out_duration = fade_out_duration;
}

void UiFadeImage::play()
{
    _elapsed = 0.0;

    switch (_mode)
    {
    case UiFadeImageMode::FadeIn:
    case UiFadeImageMode::FadeInOut:
        _alpha = 0;
        _state = FadeState::FadingIn;
        break;

    case UiFadeImageMode::FadeOut:
        _alpha = 255;
        start_hold();
        break;

    default:
        break;
    }
}

void UiFadeImage::update(double delta)
{
    if (_state == FadeState::Idle)
        return;

    switch (_state)
    {
    case FadeState::FadingIn:
        update_fade_in(delta);
        break;

    case FadeState::Holding:
        _timer.update(delta);
        break;

    case FadeState::FadingOut:
        update_fade_out(delta);
        break;

    default:
        break;
    }

    if (_state == FadeState::Finished)
    {
        if (_on_end)
            _on_end();
        UiElement::destroy();
    }
}

void  UiFadeImage::set_on_end(FadeImageOnEnd on_end)
{
    _on_end = on_end;
}



void UiFadeImage::submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const
{
    if (!_texture || !is_visible())
        return;

    out_commands.push_back(make_ui_texture_command(_texture, screen_rect(), _alpha));
}

void UiFadeImage::update_fade_in(double delta)
{
    _elapsed += delta;

    double t = ratio(_elapsed, _fade_in_duration);
    _alpha = static_cast<Uint8>(255.0 * t);

    if (t >= 1.0)
    {
        _elapsed = 0.0;
        _alpha = 255;
        start_hold();
    }
}

void UiFadeImage::update_fade_out(double delta)
{
    _elapsed += delta;

    double t = ratio(_elapsed, _fade_out_duration);
    _alpha = static_cast<Uint8>(255.0 * (1.0 - t));

    if (t >= 1.0)
    {
        _alpha = 0;
        _state = FadeState::Finished;
    }
}

double UiFadeImage::ratio(double value, double max_value) const
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

void UiFadeImage::start_hold()
{
    if (_hold_time <= 0.0)
    {
        switch (_mode)
        {
        case UiFadeImageMode::FadeIn:
            _state = FadeState::Finished;
            break;
        case UiFadeImageMode::FadeOut:
        case UiFadeImageMode::FadeInOut:
            _elapsed = 0.0;
            _state = FadeState::FadingOut;
            break;
        default:
            break;
        }
        return;
    }

    _state = FadeState::Holding;
    _timer.restart();
}
