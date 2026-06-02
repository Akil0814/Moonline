#include "ui_scroll_panel.h"

#include "../ui_mouse_utils.h"

#include <algorithm>

UiScrollPanel::UiScrollPanel(Vector2 position, Vector2 size, int order)
    : UiPanel(position, size, order)
{
    set_clip_children(true);
}

void UiScrollPanel::on_update(double delta)
{
    clamp_scroll_offset();
    set_content_offset(_scroll_offset);
    UiPanel::on_update(delta);
}

void UiScrollPanel::on_input_event(const InputEvent& event)
{
    if (event.type == InputEventType::MouseWheel)
    {
        const SDL_Point mouse_position = ui_logical_mouse_position();
        const int mouse_x = mouse_position.x;
        const int mouse_y = mouse_position.y;
        if (contains_point(mouse_x, mouse_y))
        {
            scroll_by({
                -static_cast<float>(event.wheel_x) * _scroll_step.x,
                -static_cast<float>(event.wheel_y) * _scroll_step.y
            });
        }
    }
}

void UiScrollPanel::reset()
{
    UiPanel::reset();
    set_clip_children(true);
    _scroll_offset = Vector2::zero();
    _scroll_step = { 32.0f, 32.0f };
    _allow_horizontal_scroll = true;
    _allow_vertical_scroll = true;
    _clamp_scroll = true;
    set_content_offset(_scroll_offset);
}

void UiScrollPanel::set_scroll_offset(const Vector2& offset)
{
    _scroll_offset = offset;
    clamp_scroll_offset();
    set_content_offset(_scroll_offset);
}

const Vector2& UiScrollPanel::scroll_offset() const
{
    return _scroll_offset;
}

void UiScrollPanel::scroll_by(const Vector2& delta)
{
    set_scroll_offset(_scroll_offset + delta);
}

void UiScrollPanel::set_scroll_step(const Vector2& step)
{
    _scroll_step.x = std::max(0.0f, step.x);
    _scroll_step.y = std::max(0.0f, step.y);
}

const Vector2& UiScrollPanel::scroll_step() const
{
    return _scroll_step;
}

void UiScrollPanel::scroll_left()
{
    scroll_by({ -_scroll_step.x, 0.0f });
}

void UiScrollPanel::scroll_right()
{
    scroll_by({ _scroll_step.x, 0.0f });
}

void UiScrollPanel::scroll_up()
{
    scroll_by({ 0.0f, -_scroll_step.y });
}

void UiScrollPanel::scroll_down()
{
    scroll_by({ 0.0f, _scroll_step.y });
}

void UiScrollPanel::set_allow_horizontal_scroll(bool allow_horizontal_scroll)
{
    _allow_horizontal_scroll = allow_horizontal_scroll;
    clamp_scroll_offset();
    set_content_offset(_scroll_offset);
}

bool UiScrollPanel::allows_horizontal_scroll() const
{
    return _allow_horizontal_scroll;
}

void UiScrollPanel::set_allow_vertical_scroll(bool allow_vertical_scroll)
{
    _allow_vertical_scroll = allow_vertical_scroll;
    clamp_scroll_offset();
    set_content_offset(_scroll_offset);
}

bool UiScrollPanel::allows_vertical_scroll() const
{
    return _allow_vertical_scroll;
}

void UiScrollPanel::set_clamp_scroll(bool clamp_scroll)
{
    _clamp_scroll = clamp_scroll;
    clamp_scroll_offset();
    set_content_offset(_scroll_offset);
}

bool UiScrollPanel::clamps_scroll() const
{
    return _clamp_scroll;
}

Vector2 UiScrollPanel::max_scroll_offset()
{
    relayout();

    const Vector2 content_size = measure_content_size();
    const Vector2 view_size = content_view_size();

    Vector2 max_scroll{
        std::max(0.0f, content_size.x - view_size.x),
        std::max(0.0f, content_size.y - view_size.y)
    };

    if (!_allow_horizontal_scroll)
    {
        max_scroll.x = 0.0f;
    }

    if (!_allow_vertical_scroll)
    {
        max_scroll.y = 0.0f;
    }

    return max_scroll;
}

void UiScrollPanel::ensure_child_visible(const GameObject* child)
{
    if (!child)
    {
        return;
    }

    relayout();

    SDL_Rect child_rect{};
    if (!try_get_child_rect(child, child_rect))
    {
        return;
    }

    const SDL_Rect& panel_rect = rect();

    Vector2 next_scroll = _scroll_offset;
    if (_allow_horizontal_scroll)
    {
        const int visible_left = panel_rect.x;
        const int visible_right = panel_rect.x + panel_rect.w;
        const int child_left = child_rect.x;
        const int child_right = child_rect.x + child_rect.w;

        if (child_left < visible_left)
        {
            next_scroll.x -= static_cast<float>(visible_left - child_left);
        }
        else if (child_right > visible_right)
        {
            next_scroll.x += static_cast<float>(child_right - visible_right);
        }
    }

    if (_allow_vertical_scroll)
    {
        const int visible_top = panel_rect.y;
        const int visible_bottom = panel_rect.y + panel_rect.h;
        const int child_top = child_rect.y;
        const int child_bottom = child_rect.y + child_rect.h;

        if (child_top < visible_top)
        {
            next_scroll.y -= static_cast<float>(visible_top - child_top);
        }
        else if (child_bottom > visible_bottom)
        {
            next_scroll.y += static_cast<float>(child_bottom - visible_bottom);
        }
    }

    set_scroll_offset(next_scroll);
    relayout();
}

void UiScrollPanel::clamp_scroll_offset()
{
    if (!_allow_horizontal_scroll)
    {
        _scroll_offset.x = 0.0f;
    }

    if (!_allow_vertical_scroll)
    {
        _scroll_offset.y = 0.0f;
    }

    if (!_clamp_scroll)
    {
        _scroll_offset.x = std::max(0.0f, _scroll_offset.x);
        _scroll_offset.y = std::max(0.0f, _scroll_offset.y);
        return;
    }

    const Vector2 max_scroll = max_scroll_offset();
    _scroll_offset.x = std::clamp(_scroll_offset.x, 0.0f, max_scroll.x);
    _scroll_offset.y = std::clamp(_scroll_offset.y, 0.0f, max_scroll.y);
}

bool UiScrollPanel::contains_point(int x, int y) const
{
    const SDL_Point point{ x, y };
    return SDL_PointInRect(&point, &rect()) == SDL_TRUE;
}
