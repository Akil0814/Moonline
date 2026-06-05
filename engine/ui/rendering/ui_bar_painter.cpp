#include "ui_bar_painter.h"

#include <algorithm>

void UiBarPainter::reset()
{
    _min_value = 0.0f;
    _max_value = 1.0f;
    _value = 0.0f;
    _background_color = SDL_Color{ 0, 43, 100, 255 };
    _fill_color = SDL_Color{ 245, 255, 255, 255 };
    _border_color = SDL_Color{ 0, 0, 0, 255 };
    _fill_direction = BarFillDirection::LeftToRight;
    _draw_border = false;
    _padding = 0;
}

void UiBarPainter::set_range(float min_value, float max_value)
{
    if (max_value < min_value)
    {
        std::swap(min_value, max_value);
    }

    _min_value = min_value;
    _max_value = max_value;
    _value = std::clamp(_value, _min_value, _max_value);
}

void UiBarPainter::set_value(float value)
{
    _value = std::clamp(value, _min_value, _max_value);
}

void UiBarPainter::set_ratio(float ratio)
{
    const float clamped_ratio = std::clamp(ratio, 0.0f, 1.0f);
    _value = _min_value + (_max_value - _min_value) * clamped_ratio;
}

float UiBarPainter::min_value() const
{
    return _min_value;
}

float UiBarPainter::max_value() const
{
    return _max_value;
}

float UiBarPainter::value() const
{
    return _value;
}

float UiBarPainter::ratio() const
{
    const float range = _max_value - _min_value;
    if (range <= 0.0f)
    {
        return 0.0f;
    }

    return (_value - _min_value) / range;
}

void UiBarPainter::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color UiBarPainter::background_color() const
{
    return _background_color;
}

void UiBarPainter::set_fill_color(SDL_Color color)
{
    _fill_color = color;
}

SDL_Color UiBarPainter::fill_color() const
{
    return _fill_color;
}

void UiBarPainter::set_border_color(SDL_Color color)
{
    _border_color = color;
}

SDL_Color UiBarPainter::border_color() const
{
    return _border_color;
}

void UiBarPainter::set_draw_border(bool draw_border)
{
    _draw_border = draw_border;
}

bool UiBarPainter::draws_border() const
{
    return _draw_border;
}

void UiBarPainter::set_fill_direction(BarFillDirection direction)
{
    _fill_direction = direction;
}

BarFillDirection UiBarPainter::fill_direction() const
{
    return _fill_direction;
}

void UiBarPainter::set_padding(int padding)
{
    _padding = std::max(0, padding);
}

int UiBarPainter::padding() const
{
    return _padding;
}

void UiBarPainter::render(SDL_Renderer* renderer, const SDL_Rect& rect) const
{
    if (!renderer || rect.w <= 0 || rect.h <= 0)
    {
        return;
    }

    Uint8 old_r = 0;
    Uint8 old_g = 0;
    Uint8 old_b = 0;
    Uint8 old_a = 0;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);

    SDL_SetRenderDrawColor(
        renderer,
        _background_color.r,
        _background_color.g,
        _background_color.b,
        _background_color.a
    );
    SDL_RenderFillRect(renderer, &rect);

    const SDL_Rect fill = fill_rect(rect);
    if (fill.w > 0 && fill.h > 0)
    {
        SDL_SetRenderDrawColor(
            renderer,
            _fill_color.r,
            _fill_color.g,
            _fill_color.b,
            _fill_color.a
        );
        SDL_RenderFillRect(renderer, &fill);
    }

    if (_draw_border)
    {
        SDL_SetRenderDrawColor(
            renderer,
            _border_color.r,
            _border_color.g,
            _border_color.b,
            _border_color.a
        );
        SDL_RenderDrawRect(renderer, &rect);
    }

    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

SDL_Rect UiBarPainter::content_rect(const SDL_Rect& rect) const
{
    SDL_Rect content = rect;
    const int horizontal_padding = std::min(_padding * 2, std::max(0, rect.w));
    const int vertical_padding = std::min(_padding * 2, std::max(0, rect.h));

    content.x += std::min(_padding, rect.w / 2);
    content.y += std::min(_padding, rect.h / 2);
    content.w = std::max(0, rect.w - horizontal_padding);
    content.h = std::max(0, rect.h - vertical_padding);
    return content;
}

SDL_Rect UiBarPainter::fill_rect(const SDL_Rect& rect) const
{
    SDL_Rect fill = content_rect(rect);
    const float clamped_ratio = std::clamp(ratio(), 0.0f, 1.0f);

    switch (_fill_direction)
    {
    case BarFillDirection::LeftToRight:
        fill.w = static_cast<int>(fill.w * clamped_ratio);
        break;

    case BarFillDirection::RightToLeft:
    {
        const int new_width = static_cast<int>(fill.w * clamped_ratio);
        fill.x += fill.w - new_width;
        fill.w = new_width;
        break;
    }

    case BarFillDirection::TopToBottom:
        fill.h = static_cast<int>(fill.h * clamped_ratio);
        break;

    case BarFillDirection::BottomToTop:
    {
        const int new_height = static_cast<int>(fill.h * clamped_ratio);
        fill.y += fill.h - new_height;
        fill.h = new_height;
        break;
    }
    }

    return fill;
}
