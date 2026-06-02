#include "ui_scroll_bar.h"

#include "../ui_mouse_utils.h"
#include "../style/ui_theme.h"
#include "../style/ui_style.h"

#include <algorithm>

UiScrollBar::UiScrollBar(Vector2 position, Vector2 size, int order)
    : UiElement(position, size, order)
{
}

void UiScrollBar::on_update(double delta)
{
    refresh_theme_if_needed();
    (void)delta;
    sync_with_target();
}

void UiScrollBar::on_render(SDL_Renderer* renderer)
{
    if (!renderer || !should_draw())
    {
        return;
    }

    refresh_theme_if_needed();
    const SDL_Rect track = track_rect();
    const SDL_Rect thumb = thumb_rect();
    if (track.w <= 0 || track.h <= 0 || thumb.w <= 0 || thumb.h <= 0)
    {
        return;
    }

    Uint8 old_r = 0;
    Uint8 old_g = 0;
    Uint8 old_b = 0;
    Uint8 old_a = 0;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);

    if (_draw_track)
    {
        SDL_SetRenderDrawColor(
            renderer,
            _track_color.r,
            _track_color.g,
            _track_color.b,
            _track_color.a
        );
        SDL_RenderFillRect(renderer, &track);
    }

    SDL_SetRenderDrawColor(
        renderer,
        _thumb_color.r,
        _thumb_color.g,
        _thumb_color.b,
        _thumb_color.a
    );
    SDL_RenderFillRect(renderer, &thumb);

    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

void UiScrollBar::on_input(const InputSnapshot& input)
{
    if (!_target || !should_draw() || !ui_mouse_input_allowed(input))
    {
        ui_reset_mouse_drag_state(_is_dragging, _was_mouse_down);
        return;
    }

    const UiMouseState mouse_state = ui_current_mouse_state();
    const int mouse_x = mouse_state._position.x;
    const int mouse_y = mouse_state._position.y;

    const SDL_Rect current_track = track_rect();
    const SDL_Rect current_thumb = thumb_rect();

    if (ui_begin_mouse_drag(_is_dragging, mouse_state, _was_mouse_down, current_thumb))
    {
        _drag_offset = _orientation == ScrollBarOrientation::Vertical
            ? static_cast<float>(mouse_y - current_thumb.y)
            : static_cast<float>(mouse_x - current_thumb.x);
    }
    else if (ui_left_mouse_pressed_in_rect(mouse_state, _was_mouse_down, current_track))
    {
        update_target_from_point(mouse_x, mouse_y);
    }
    else if (ui_drag_in_progress(mouse_state, _is_dragging))
    {
        update_target_from_point(mouse_x, mouse_y);
    }

    ui_sync_mouse_drag_state(_is_dragging, _was_mouse_down, mouse_state);
}

void UiScrollBar::reset()
{
    UiElement::reset();
    _target = nullptr;
    _track_color = SDL_Color{ 32, 40, 56, 180 };
    _thumb_color = SDL_Color{ 126, 156, 196, 255 };
    _target_margin = 6.0f;
    _thickness = 8.0f;
    _min_thumb_size = 24.0f;
    _drag_offset = 0.0f;
    _auto_follow_target = true;
    _draw_track = true;
    _auto_hide = true;
    ui_reset_mouse_drag_state(_is_dragging, _was_mouse_down);
    _orientation = ScrollBarOrientation::Vertical;
    mark_theme_dirty();
}

void UiScrollBar::set_target(UiScrollPanel* target)
{
    _target = target;
    sync_with_target();
}

UiScrollPanel* UiScrollBar::target() const
{
    return _target;
}

void UiScrollBar::set_orientation(ScrollBarOrientation orientation)
{
    _orientation = orientation;
    sync_with_target();
}

ScrollBarOrientation UiScrollBar::orientation() const
{
    return _orientation;
}

void UiScrollBar::set_auto_follow_target(bool auto_follow_target)
{
    _auto_follow_target = auto_follow_target;
    sync_with_target();
}

bool UiScrollBar::auto_follows_target() const
{
    return _auto_follow_target;
}

void UiScrollBar::set_target_margin(float margin)
{
    _target_margin = std::max(0.0f, margin);
    sync_with_target();
}

float UiScrollBar::target_margin() const
{
    return _target_margin;
}

void UiScrollBar::set_thickness(float thickness)
{
    _thickness = std::max(1.0f, thickness);
    sync_with_target();
}

float UiScrollBar::thickness() const
{
    return _thickness;
}

void UiScrollBar::set_min_thumb_size(float min_thumb_size)
{
    _min_thumb_size = std::max(1.0f, min_thumb_size);
}

float UiScrollBar::min_thumb_size() const
{
    return _min_thumb_size;
}

void UiScrollBar::set_track_color(SDL_Color color)
{
    _track_color = color;
}

SDL_Color UiScrollBar::track_color() const
{
    return _track_color;
}

void UiScrollBar::set_thumb_color(SDL_Color color)
{
    _thumb_color = color;
}

SDL_Color UiScrollBar::thumb_color() const
{
    return _thumb_color;
}

void UiScrollBar::set_draw_track(bool draw_track)
{
    _draw_track = draw_track;
}

bool UiScrollBar::draws_track() const
{
    return _draw_track;
}

void UiScrollBar::set_auto_hide(bool auto_hide)
{
    _auto_hide = auto_hide;
}

bool UiScrollBar::auto_hides() const
{
    return _auto_hide;
}

void UiScrollBar::sync_with_target()
{
    if (!_target || !_auto_follow_target)
    {
        return;
    }

    const SDL_Rect& target_rect = _target->rect();
    if (_orientation == ScrollBarOrientation::Vertical)
    {
        GameObject::set_world_position({
            static_cast<float>(target_rect.x + target_rect.w + static_cast<int>(_target_margin)),
            static_cast<float>(target_rect.y)
        });
        GameObject::set_size({ _thickness, static_cast<float>(target_rect.h) });
        return;
    }

    GameObject::set_world_position({
        static_cast<float>(target_rect.x),
        static_cast<float>(target_rect.y + target_rect.h + static_cast<int>(_target_margin))
    });
    GameObject::set_size({ static_cast<float>(target_rect.w), _thickness });
}

SDL_Rect UiScrollBar::track_rect() const
{
    return rect();
}

SDL_Rect UiScrollBar::thumb_rect() const
{
    SDL_Rect thumb = rect();
    if (!_target)
    {
        thumb.w = 0;
        thumb.h = 0;
        return thumb;
    }

    const Vector2 content_size = const_cast<UiScrollPanel*>(_target)->measure_content_size();
    const Vector2 view_size = _target->content_view_size();
    const Vector2 scroll_offset = _target->scroll_offset();
    const Vector2 max_scroll = const_cast<UiScrollPanel*>(_target)->max_scroll_offset();

    if (_orientation == ScrollBarOrientation::Vertical)
    {
        if (view_size.y <= 0.0f || content_size.y <= 0.0f)
        {
            thumb.h = 0;
            return thumb;
        }

        const float visible_ratio = std::clamp(view_size.y / content_size.y, 0.0f, 1.0f);
        const float track_height = static_cast<float>(thumb.h);
        const float thumb_height = std::max(_min_thumb_size, track_height * visible_ratio);
        const float scroll_ratio = max_scroll.y > 0.0f
            ? std::clamp(scroll_offset.y / max_scroll.y, 0.0f, 1.0f)
            : 0.0f;
        const float travel = std::max(0.0f, track_height - thumb_height);

        thumb.y += static_cast<int>(travel * scroll_ratio);
        thumb.h = static_cast<int>(thumb_height);
        return thumb;
    }

    if (view_size.x <= 0.0f || content_size.x <= 0.0f)
    {
        thumb.w = 0;
        return thumb;
    }

    const float visible_ratio = std::clamp(view_size.x / content_size.x, 0.0f, 1.0f);
    const float track_width = static_cast<float>(thumb.w);
    const float thumb_width = std::max(_min_thumb_size, track_width * visible_ratio);
    const float scroll_ratio = max_scroll.x > 0.0f
        ? std::clamp(scroll_offset.x / max_scroll.x, 0.0f, 1.0f)
        : 0.0f;
    const float travel = std::max(0.0f, track_width - thumb_width);

    thumb.x += static_cast<int>(travel * scroll_ratio);
    thumb.w = static_cast<int>(thumb_width);
    return thumb;
}

bool UiScrollBar::should_draw() const
{
    if (!_target)
    {
        return false;
    }

    const Vector2 content_size = const_cast<UiScrollPanel*>(_target)->measure_content_size();
    const Vector2 view_size = _target->content_view_size();

    if (_orientation == ScrollBarOrientation::Vertical)
    {
        return !_auto_hide || content_size.y > view_size.y;
    }

    return !_auto_hide || content_size.x > view_size.x;
}

void UiScrollBar::update_target_from_point(int x, int y)
{
    if (!_target)
    {
        return;
    }

    const SDL_Rect current_track = track_rect();
    const SDL_Rect current_thumb = thumb_rect();
    const Vector2 max_scroll = _target->max_scroll_offset();
    Vector2 next_offset = _target->scroll_offset();

    if (_orientation == ScrollBarOrientation::Vertical)
    {
        const float thumb_height = static_cast<float>(std::max(1, current_thumb.h));
        const float travel = static_cast<float>(std::max(1, current_track.h - current_thumb.h));
        const float thumb_top = _is_dragging
            ? static_cast<float>(y) - _drag_offset
            : static_cast<float>(y) - thumb_height * 0.5f;
        const float clamped_top = std::clamp(thumb_top, static_cast<float>(current_track.y), static_cast<float>(current_track.y) + travel);
        const float ratio = travel > 0.0f
            ? (clamped_top - static_cast<float>(current_track.y)) / travel
            : 0.0f;
        next_offset.y = max_scroll.y * ratio;
    }
    else
    {
        const float thumb_width = static_cast<float>(std::max(1, current_thumb.w));
        const float travel = static_cast<float>(std::max(1, current_track.w - current_thumb.w));
        const float thumb_left = _is_dragging
            ? static_cast<float>(x) - _drag_offset
            : static_cast<float>(x) - thumb_width * 0.5f;
        const float clamped_left = std::clamp(thumb_left, static_cast<float>(current_track.x), static_cast<float>(current_track.x) + travel);
        const float ratio = travel > 0.0f
            ? (clamped_left - static_cast<float>(current_track.x)) / travel
            : 0.0f;
        next_offset.x = max_scroll.x * ratio;
    }

    _target->set_scroll_offset(next_offset);
}

void UiScrollBar::apply_theme(const UiTheme& theme)
{
    UiStyle::apply_scroll_bar(*this, theme._scroll_bar);
}
