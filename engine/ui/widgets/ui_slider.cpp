#include "ui_slider.h"

#include "../ui_mouse_utils.h"
#include "../style/ui_theme.h"
#include "../style/ui_style.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

UiSlider::UiSlider(Vector2 position, Vector2 size, int order)
    : UiControl(position, size, order), _value_label(Vector2::zero(), Vector2::zero(), order + 1)
{
    reset();
}

void UiSlider::set_world_position(const Vector2& position)
{
    GameObject::set_world_position(position);
    sync_value_label();
}

void UiSlider::set_size(const Vector2& size)
{
    GameObject::set_size(size);
    sync_value_label();
}

void UiSlider::on_update(double delta)
{
    refresh_theme_if_needed();
    (void)delta;
    _value_label.on_update(delta);
}

void UiSlider::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    refresh_theme_if_needed();
    const SDL_Rect slider_rect = rect();
    if (slider_rect.w <= 0 || slider_rect.h <= 0)
    {
        return;
    }

    const SDL_Rect track = track_rect();
    _bar.render(renderer, track);

    const SDL_Rect thumb = thumb_rect();
    if (thumb.w > 0 && thumb.h > 0)
    {
        Uint8 old_r = 0;
        Uint8 old_g = 0;
        Uint8 old_b = 0;
        Uint8 old_a = 0;
        SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);

        SDL_Color draw_color = _thumb_color;
        if (_is_focused)
        {
            draw_color.r = static_cast<Uint8>(std::min(255, draw_color.r + 18));
            draw_color.g = static_cast<Uint8>(std::min(255, draw_color.g + 18));
            draw_color.b = static_cast<Uint8>(std::min(255, draw_color.b + 18));
        }

        SDL_SetRenderDrawColor(renderer, draw_color.r, draw_color.g, draw_color.b, draw_color.a);
        SDL_RenderFillRect(renderer, &thumb);
        SDL_SetRenderDrawColor(renderer, 18, 22, 28, 255);
        SDL_RenderDrawRect(renderer, &thumb);
        SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
    }

    if (_show_value_text)
    {
        _value_label.on_render(renderer);
    }
}

void UiSlider::on_input(const InputSnapshot& input)
{
    if (!_enabled || !ui_mouse_input_allowed(input))
    {
        _is_dragging = false;
        _was_mouse_down = false;
        return;
    }

    const UiMouseState mouse_state = ui_current_mouse_state();
    const int mouse_x = mouse_state._position.x;
    const int mouse_y = mouse_state._position.y;

    if (ui_left_mouse_pressed(mouse_state, _was_mouse_down) && ui_contains_point(rect(), mouse_state._position))
    {
        _is_dragging = true;
        update_value_from_point(mouse_x, mouse_y, true);
    }
    else if (mouse_state._left_button_down && _is_dragging)
    {
        update_value_from_point(mouse_x, mouse_y, true);
    }
    else if (!mouse_state._left_button_down)
    {
        _is_dragging = false;
    }

    _was_mouse_down = mouse_state._left_button_down;
}

void UiSlider::reset()
{
    UiControl::reset();
    _bar.reset();
    _bar.set_range(0.0f, 100.0f);
    _bar.set_value(50.0f);
    _bar.set_padding(4);
    _bar.set_draw_border(true);

    _value_label.reset();
    _value_label.set_font_key(_font_key);
    _value_label.set_text_color(_text_color);
    _value_label.set_horizontal_align(TextHorizontalAlign::Right);
    _value_label.set_vertical_align(TextVerticalAlign::Center);
    _value_label.set_auto_size(false);

    _on_value_changed = nullptr;
    _value_suffix.clear();
    _step = 1.0f;
    _thumb_size = 16.0f;
    _value_precision = 0;
    _is_dragging = false;
    _was_mouse_down = false;
    _show_value_text = false;
    _orientation = UiSliderOrientation::Horizontal;
    _thumb_color = SDL_Color{ 232, 240, 250, 255 };
    _text_color = SDL_Color{ 244, 244, 248, 255 };

    sync_value_label();
    mark_theme_dirty();
}

void UiSlider::set_range(float min_value, float max_value)
{
    _bar.set_range(min_value, max_value);
    set_value_internal(_bar.value(), false);
}

float UiSlider::min_value() const
{
    return _bar.min_value();
}

float UiSlider::max_value() const
{
    return _bar.max_value();
}

void UiSlider::set_value(float value)
{
    set_value_internal(value, true);
}

float UiSlider::value() const
{
    return _bar.value();
}

void UiSlider::set_step(float step)
{
    _step = step > 0.0f ? step : 1.0f;
    set_value_internal(_bar.value(), false);
}

float UiSlider::step() const
{
    return _step;
}

void UiSlider::set_orientation(UiSliderOrientation orientation)
{
    _orientation = orientation;
}

UiSliderOrientation UiSlider::orientation() const
{
    return _orientation;
}

UiBar& UiSlider::bar()
{
    return _bar;
}

const UiBar& UiSlider::bar() const
{
    return _bar;
}

void UiSlider::set_thumb_color(SDL_Color color)
{
    _thumb_color = color;
}

SDL_Color UiSlider::thumb_color() const
{
    return _thumb_color;
}

void UiSlider::set_thumb_size(float thumb_size)
{
    _thumb_size = std::max(4.0f, thumb_size);
}

float UiSlider::thumb_size() const
{
    return _thumb_size;
}

void UiSlider::set_show_value_text(bool show_value_text)
{
    _show_value_text = show_value_text;
    sync_value_label();
}

bool UiSlider::shows_value_text() const
{
    return _show_value_text;
}

void UiSlider::set_value_precision(int precision)
{
    _value_precision = std::max(0, precision);
    sync_value_label();
}

int UiSlider::value_precision() const
{
    return _value_precision;
}

void UiSlider::set_value_suffix(const std::string& suffix)
{
    _value_suffix = suffix;
    sync_value_label();
}

const std::string& UiSlider::value_suffix() const
{
    return _value_suffix;
}

void UiSlider::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    _value_label.set_font_key(font_key);
}

const std::string& UiSlider::font_key() const
{
    return _font_key;
}

void UiSlider::set_text_color(SDL_Color color)
{
    _text_color = color;
    _value_label.set_text_color(color);
}

SDL_Color UiSlider::text_color() const
{
    return _text_color;
}

void UiSlider::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    if (!_enabled)
    {
        _is_dragging = false;
        _was_mouse_down = false;
    }
}

void UiSlider::set_on_value_changed(UiSliderValueChangedCallback on_value_changed)
{
    _on_value_changed = std::move(on_value_changed);
}

bool UiSlider::handle_focused_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused || event.type != InputEventType::Pressed)
    {
        return false;
    }

    if (_orientation == UiSliderOrientation::Horizontal)
    {
        if (event.action == InputAction::Left)
        {
            set_value_internal(_bar.value() - _step, true);
            return true;
        }

        if (event.action == InputAction::Right)
        {
            set_value_internal(_bar.value() + _step, true);
            return true;
        }
    }
    else
    {
        if (event.action == InputAction::Down)
        {
            set_value_internal(_bar.value() - _step, true);
            return true;
        }

        if (event.action == InputAction::Up)
        {
            set_value_internal(_bar.value() + _step, true);
            return true;
        }
    }

    return false;
}

void UiSlider::sync_value_label()
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(_value_precision) << _bar.value();
    if (!_value_suffix.empty())
    {
        stream << _value_suffix;
    }

    _value_label.set_font_key(_font_key);
    _value_label.set_text_color(_text_color);
    _value_label.set_text(stream.str());
    _value_label.set_world_position(position());
    _value_label.set_size(size());
}

void UiSlider::set_value_internal(float value, bool notify)
{
    const float previous_value = _bar.value();
    _bar.set_value(snapped_value(value));
    sync_value_label();

    if (notify && std::fabs(previous_value - _bar.value()) > 0.0001f && _on_value_changed)
    {
        _on_value_changed(_bar.value());
    }
}

float UiSlider::snapped_value(float value) const
{
    const float min_value = _bar.min_value();
    const float max_value = _bar.max_value();
    const float step_value = _step > 0.0f ? _step : 1.0f;
    const float offset = value - min_value;
    const float steps = std::round(offset / step_value);
    return std::clamp(min_value + steps * step_value, min_value, max_value);
}

SDL_Rect UiSlider::track_rect() const
{
    SDL_Rect track = rect();
    if (_orientation == UiSliderOrientation::Horizontal)
    {
        track.y += track.h / 2 - 6;
        track.h = std::min(track.h, 12);
    }
    else
    {
        track.x += track.w / 2 - 6;
        track.w = std::min(track.w, 12);
    }

    return track;
}

SDL_Rect UiSlider::thumb_rect() const
{
    SDL_Rect track = track_rect();
    const float ratio = _bar.ratio();
    SDL_Rect thumb{};

    if (_orientation == UiSliderOrientation::Horizontal)
    {
        const int track_left = track.x + _bar.padding();
        const int track_right = track.x + track.w - _bar.padding();
        const int travel = std::max(0, track_right - track_left);
        const int center_x = track_left + static_cast<int>(travel * ratio);
        thumb.w = static_cast<int>(_thumb_size);
        thumb.h = static_cast<int>(std::max(_thumb_size, static_cast<float>(track.h + 8)));
        thumb.x = center_x - thumb.w / 2;
        thumb.y = track.y + (track.h - thumb.h) / 2;
    }
    else
    {
        const int track_top = track.y + _bar.padding();
        const int track_bottom = track.y + track.h - _bar.padding();
        const int travel = std::max(0, track_bottom - track_top);
        const int center_y = track_bottom - static_cast<int>(travel * ratio);
        thumb.w = static_cast<int>(std::max(_thumb_size, static_cast<float>(track.w + 8)));
        thumb.h = static_cast<int>(_thumb_size);
        thumb.x = track.x + (track.w - thumb.w) / 2;
        thumb.y = center_y - thumb.h / 2;
    }

    return thumb;
}

void UiSlider::update_value_from_point(int x, int y, bool notify)
{
    const SDL_Rect track = track_rect();
    if (_orientation == UiSliderOrientation::Horizontal)
    {
        const int left = track.x + _bar.padding();
        const int right = std::max(left, track.x + track.w - _bar.padding());
        const float ratio = right > left
            ? std::clamp(static_cast<float>(x - left) / static_cast<float>(right - left), 0.0f, 1.0f)
            : 0.0f;
        const float next_value = _bar.min_value() + (_bar.max_value() - _bar.min_value()) * ratio;
        set_value_internal(next_value, notify);
        return;
    }

    const int top = track.y + _bar.padding();
    const int bottom = std::max(top, track.y + track.h - _bar.padding());
    const float ratio = bottom > top
        ? std::clamp(static_cast<float>(bottom - y) / static_cast<float>(bottom - top), 0.0f, 1.0f)
        : 0.0f;
    const float next_value = _bar.min_value() + (_bar.max_value() - _bar.min_value()) * ratio;
    set_value_internal(next_value, notify);
}

void UiSlider::apply_theme(const UiTheme& theme)
{
    UiStyle::apply_slider(*this, theme._slider);
}
