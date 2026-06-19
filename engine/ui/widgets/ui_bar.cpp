#include "ui_bar.h"

#include "../../core/render/render_command.h"

#include <algorithm>

UiBar::UiBar(const Rect& rect, int order) noexcept
    : UiElement(rect, order)
{
    set_use_theme(false);
}

UiBar::UiBar(const Vector2& position, const Vector2& size, int order) noexcept
    : UiElement(position, size, order)
{
    set_use_theme(false);
}

void UiBar::reset() noexcept
{
    UiElement::reset();
    set_use_theme(false);

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

void UiBar::set_range(float min_value, float max_value)
{
    if (max_value < min_value)
    {
        std::swap(min_value, max_value);
    }

    _min_value = min_value;
    _max_value = max_value;
    _value = std::clamp(_value, _min_value, _max_value);
}

void UiBar::set_value(float value)
{
    _value = std::clamp(value, _min_value, _max_value);
}

void UiBar::set_ratio(float ratio)
{
    const float clamped_ratio = std::clamp(ratio, 0.0f, 1.0f);
    _value = _min_value + (_max_value - _min_value) * clamped_ratio;
}

float UiBar::min_value() const
{
    return _min_value;
}

float UiBar::max_value() const
{
    return _max_value;
}

float UiBar::value() const
{
    return _value;
}

float UiBar::ratio() const
{
    const float range = _max_value - _min_value;
    if (range <= 0.0f)
    {
        return 0.0f;
    }

    return (_value - _min_value) / range;
}

void UiBar::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color UiBar::background_color() const
{
    return _background_color;
}

void UiBar::set_fill_color(SDL_Color color)
{
    _fill_color = color;
}

SDL_Color UiBar::fill_color() const
{
    return _fill_color;
}

void UiBar::set_border_color(SDL_Color color)
{
    _border_color = color;
}

SDL_Color UiBar::border_color() const
{
    return _border_color;
}

void UiBar::set_draw_border(bool draw_border)
{
    _draw_border = draw_border;
}

bool UiBar::draws_border() const
{
    return _draw_border;
}

void UiBar::set_fill_direction(BarFillDirection direction)
{
    _fill_direction = direction;
}

BarFillDirection UiBar::fill_direction() const
{
    return _fill_direction;
}

void UiBar::set_padding(int padding)
{
    _padding = std::max(0, padding);
}

int UiBar::padding() const
{
    return _padding;
}

void UiBar::submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const
{
    if (!is_visible())
    {
        return;
    }

    const Rect& bar_rect = screen_rect();
    if (bar_rect.is_empty())
    {
        return;
    }

    out_commands.push_back(make_ui_fill_rect_command(bar_rect, _background_color));

    const Rect fill = fill_rect(bar_rect);
    if (!fill.is_empty())
    {
        out_commands.push_back(make_ui_fill_rect_command(fill, _fill_color));
    }

    if (_draw_border)
    {
        out_commands.push_back(make_ui_draw_rect_command(bar_rect, _border_color));
    }
}

Rect UiBar::content_rect(const Rect& rect) const
{
    const float width = std::max(0.0f, rect.width());
    const float height = std::max(0.0f, rect.height());

    const float padding = static_cast<float>(_padding);
    const float pad_x = std::min(padding, width * 0.5f);
    const float pad_y = std::min(padding, height * 0.5f);

    Rect content = rect;
    content.set_x(rect.x() + pad_x);
    content.set_y(rect.y() + pad_y);
    content.set_width(width - pad_x * 2.0f);
    content.set_height(height - pad_y * 2.0f);
    return content;
}

Rect UiBar::fill_rect(const Rect& rect) const
{
    Rect fill = content_rect(rect);
    const float clamped_ratio = std::clamp(ratio(), 0.0f, 1.0f);

    switch (_fill_direction)
    {
    case BarFillDirection::LeftToRight:
        fill.set_width(fill.width() * clamped_ratio);
        break;

    case BarFillDirection::RightToLeft:
    {
        const float new_width = fill.width() * clamped_ratio;
        fill.set_x(fill.x() + fill.width() - new_width);
        fill.set_width(new_width);
        break;
    }

    case BarFillDirection::TopToBottom:
        fill.set_height(fill.height() * clamped_ratio);
        break;

    case BarFillDirection::BottomToTop:
    {
        const float new_height = fill.height() * clamped_ratio;
        fill.set_y(fill.y() + fill.height() - new_height);
        fill.set_height(new_height);
        break;
    }
    }

    return fill;
}
