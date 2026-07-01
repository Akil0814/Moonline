#include "ui_scroll_container.h"

#include "../core/ui_control.h"
#include "../../core/render/render_command.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
constexpr float ScrollbarEpsilon = 0.01f;

[[nodiscard]] float clamp_non_negative(float value) noexcept
{
    return std::max(0.0f,value);
}

[[nodiscard]] float clamp01(float value) noexcept
{
    return std::clamp(value,0.0f,1.0f);
}

[[nodiscard]] bool is_horizontal_axis(UiScrollAxis axis) noexcept
{
    return axis == UiScrollAxis::Horizontal;
}
}

UiScrollContainer::UiScrollContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiChildHost(rect,order)
{
    initialize_scrollbar_handles();
}

UiScrollContainer::UiScrollContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiChildHost(position,size,order)
{
    initialize_scrollbar_handles();
}

UiScrollContainer::UiScrollContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiChildHost(center,size,from_center,order)
{
    initialize_scrollbar_handles();
}

void UiScrollContainer::reset() noexcept
{
    UiChildHost::reset();
    UiChildHost::set_clip_children(true);
    _scroll_state.reset();
    _scrollbar_visibility = UiScrollBarVisibility::Auto;
    _scrollbar_style = UiScrollBarStyle{};
    _scope_focused = false;
    initialize_scrollbar_handles();
}

bool UiScrollContainer::on_ui_input_event(const UiInputEvent& event)
{
    UiChildHost::set_clip_children(true);
    cleanup_destroyed_children();
    update_layout_if_dirty();

    if (dispatch_to_scrollbars(event))
        return true;
    if (handle_mouse_wheel(event))
        return true;

    const bool handled = UiChildHost::on_ui_input_event(event);
    ensure_visible_focused_target();
    return handled;
}

void UiScrollContainer::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    auto* self = const_cast<UiScrollContainer*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    submit_child_render_commands(out_commands);
    submit_scrollbar_render_commands(out_commands);
}

UiElement* UiScrollContainer::add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options)
{
    (void)options;
    return set_content_internal(std::move(child));
}

UiElement* UiScrollContainer::set_content(std::unique_ptr<UiElement> content)
{
    return set_content_internal(std::move(content));
}

UiElement* UiScrollContainer::content() noexcept
{
    return child_at(0);
}

const UiElement* UiScrollContainer::content() const noexcept
{
    return child_at(0);
}

void UiScrollContainer::clear_content()
{
    UiChildHost::clear_children();
    reset_scroll_offset();
}

void UiScrollContainer::set_scroll_axis(UiScrollAxis axis) noexcept
{
    _scroll_state.set_axis(axis);
    mark_layout_dirty();
}

UiScrollAxis UiScrollContainer::scroll_axis() const noexcept
{
    return _scroll_state.axis();
}

UiScrollAxis UiScrollContainer::resolved_scroll_axis() const noexcept
{
    auto* self = const_cast<UiScrollContainer*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    return _scroll_state.resolved_axis();
}

void UiScrollContainer::set_scrollbar_visibility(UiScrollBarVisibility visibility) noexcept
{
    _scrollbar_visibility = visibility;
    mark_layout_dirty();
}

UiScrollBarVisibility UiScrollContainer::scrollbar_visibility() const noexcept
{
    return _scrollbar_visibility;
}

void UiScrollContainer::set_scrollbar_style(const UiScrollBarStyle& style)
{
    _scrollbar_style = style;
    mark_layout_dirty();
}

const UiScrollBarStyle& UiScrollContainer::scrollbar_style() const noexcept
{
    return _scrollbar_style;
}

void UiScrollContainer::set_content_size(const elysia::core::Vector2& content_size) noexcept
{
    _scroll_state.set_content_size(content_size);
    mark_layout_dirty();
}

elysia::core::Vector2 UiScrollContainer::content_size() const noexcept
{
    return _scroll_state.content_size();
}

void UiScrollContainer::set_content_width(float content_width) noexcept
{
    const elysia::core::Vector2 size = _scroll_state.content_size();
    _scroll_state.set_content_size(elysia::core::Vector2(content_width,size.y));
    mark_layout_dirty();
}

float UiScrollContainer::content_width() const noexcept
{
    return _scroll_state.content_size().x;
}

void UiScrollContainer::set_content_height(float content_height) noexcept
{
    const elysia::core::Vector2 size = _scroll_state.content_size();
    _scroll_state.set_content_size(elysia::core::Vector2(size.x,content_height));
    mark_layout_dirty();
}

float UiScrollContainer::content_height() const noexcept
{
    return _scroll_state.content_size().y;
}

void UiScrollContainer::set_scroll_offset(const elysia::core::Vector2& scroll_offset) noexcept
{
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.set_offset(scroll_offset);
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

elysia::core::Vector2 UiScrollContainer::scroll_offset() const noexcept
{
    return _scroll_state.offset();
}

void UiScrollContainer::set_scroll_offset_x(float scroll_offset_x) noexcept
{
    const elysia::core::Vector2 offset = _scroll_state.offset();
    set_scroll_offset(elysia::core::Vector2(scroll_offset_x,offset.y));
}

float UiScrollContainer::scroll_offset_x() const noexcept
{
    return _scroll_state.offset().x;
}

void UiScrollContainer::set_scroll_offset_y(float scroll_offset_y) noexcept
{
    const elysia::core::Vector2 offset = _scroll_state.offset();
    set_scroll_offset(elysia::core::Vector2(offset.x,scroll_offset_y));
}

float UiScrollContainer::scroll_offset_y() const noexcept
{
    return _scroll_state.offset().y;
}

elysia::core::Vector2 UiScrollContainer::max_scroll_offset() const noexcept
{
    auto* self = const_cast<UiScrollContainer*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    return _scroll_state.max_offset();
}

void UiScrollContainer::set_scroll_step(const elysia::core::Vector2& scroll_step) noexcept
{
    _scroll_state.set_step(scroll_step);
}

elysia::core::Vector2 UiScrollContainer::scroll_step() const noexcept
{
    return _scroll_state.step();
}

void UiScrollContainer::set_scroll_step_x(float scroll_step_x) noexcept
{
    const elysia::core::Vector2 step = _scroll_state.step();
    _scroll_state.set_step(elysia::core::Vector2(scroll_step_x,step.y));
}

float UiScrollContainer::scroll_step_x() const noexcept
{
    return _scroll_state.step().x;
}

void UiScrollContainer::set_scroll_step_y(float scroll_step_y) noexcept
{
    const elysia::core::Vector2 step = _scroll_state.step();
    _scroll_state.set_step(elysia::core::Vector2(step.x,scroll_step_y));
}

float UiScrollContainer::scroll_step_y() const noexcept
{
    return _scroll_state.step().y;
}

void UiScrollContainer::scroll_by(const elysia::core::Vector2& delta) noexcept
{
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.scroll_by(delta);
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::scroll_to_left() noexcept
{
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.scroll_to_left();
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::scroll_to_right() noexcept
{
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.scroll_to_right();
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::scroll_to_top() noexcept
{
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.scroll_to_top();
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::scroll_to_bottom() noexcept
{
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.scroll_to_bottom();
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::ensure_visible(const elysia::core::Rect& target_rect) noexcept
{
    UiElement* content_element = content();
    if (!content_element || target_rect.is_empty())
        return;

    const elysia::core::Rect content_rect = content_element->screen_rect();
    const elysia::core::Rect local_rect(
        target_rect.x() - content_rect.x(),
        target_rect.y() - content_rect.y(),
        target_rect.width(),
        target_rect.height()
    );

    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.ensure_visible(local_rect);
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

UiElement& UiScrollContainer::focus_scope_element() noexcept
{
    return *this;
}

const UiElement& UiScrollContainer::focus_scope_element() const noexcept
{
    return *this;
}

void UiScrollContainer::set_scope_focused(bool focused) noexcept
{
    _scope_focused = focused;
    if (UiFocusScope* scope = content_scope())
        scope->set_scope_focused(focused);
    if (focused)
        ensure_visible_focused_target();
}

bool UiScrollContainer::is_scope_focused() const noexcept
{
    return _scope_focused;
}

bool UiScrollContainer::has_focusable_target() const noexcept
{
    const UiFocusScope* scope = content_scope();
    return scope && scope->has_focusable_target();
}

bool UiScrollContainer::focus_first_available()
{
    if (UiFocusScope* scope = content_scope())
    {
        const bool focused = scope->focus_first_available();
        if (focused)
            ensure_visible_focused_target();
        return focused;
    }
    return false;
}

UiControl* UiScrollContainer::focused_target() const noexcept
{
    if (const UiFocusScope* scope = content_scope())
        return scope->focused_target();
    return nullptr;
}

bool UiScrollContainer::contains_focus_point(int mouse_x,int mouse_y) const noexcept
{
    if (!has_focusable_target() || is_destroyed() || !is_active() || !is_visible())
        return false;
    return content_rect().contains(elysia::core::Vector2(static_cast<float>(mouse_x),static_cast<float>(mouse_y)));
}

void UiScrollContainer::rebuild_layout()
{
    UiChildHost::set_clip_children(true);
    sync_scroll_state_to_viewport();

    std::vector<ChildEntry>& child_entries = children();
    const elysia::core::Rect viewport = content_rect();
    const elysia::core::Vector2 size = _scroll_state.effective_content_size();
    for (std::size_t index = 0; index < child_entries.size(); ++index)
    {
        ChildEntry& entry = child_entries[index];
        if (!entry.element)
            continue;

        if (index == 0)
        {
            entry.element->set_screen_rect(elysia::core::Rect(
                viewport.x() - _scroll_state.offset().x,
                viewport.y() - _scroll_state.offset().y,
                size.x,
                size.y
            ));
        }
        else
        {
            entry.element->set_screen_rect(elysia::core::Rect::zero());
        }
    }

    sync_scrollbar_handles();
}

UiFocusScope* UiScrollContainer::content_scope() noexcept
{
    return dynamic_cast<UiFocusScope*>(content());
}

const UiFocusScope* UiScrollContainer::content_scope() const noexcept
{
    return dynamic_cast<const UiFocusScope*>(content());
}

UiElement* UiScrollContainer::set_content_internal(std::unique_ptr<UiElement> content)
{
    UiChildHost::clear_children();
    reset_scroll_offset();

    if (!content)
        return nullptr;

    UiElement* element = UiChildHost::insert_child(std::move(content),0,UiLayoutChildOptions{});
    if (_scope_focused)
        set_scope_focused(true);
    ensure_visible_focused_target();
    return element;
}

bool UiScrollContainer::handle_mouse_wheel(const UiInputEvent& event)
{
    if (event.type != UiInputEventType::MouseWheel)
        return false;

    const elysia::core::Rect viewport = content_rect();
    const elysia::core::Vector2 point(static_cast<float>(event.mouse_x),static_cast<float>(event.mouse_y));
    if (!viewport.contains(point))
        return false;

    const elysia::core::Vector2 before = _scroll_state.offset();
    if (event.wheel_x != 0 && _scroll_state.can_scroll_horizontal())
        _scroll_state.scroll_by(elysia::core::Vector2(-static_cast<float>(event.wheel_x) * _scroll_state.step().x,0.0f));

    if (event.wheel_y != 0)
    {
        if (_scroll_state.can_scroll_vertical())
            _scroll_state.scroll_by(elysia::core::Vector2(0.0f,-static_cast<float>(event.wheel_y) * _scroll_state.step().y));
        else if (_scroll_state.can_scroll_horizontal())
            _scroll_state.scroll_by(elysia::core::Vector2(-static_cast<float>(event.wheel_y) * _scroll_state.step().x,0.0f));
    }

    if (_scroll_state.offset().nearly_equals(before))
        return false;

    mark_layout_dirty();
    return true;
}

bool UiScrollContainer::dispatch_to_scrollbars(const UiInputEvent& event)
{
    if (_vertical_thumb.is_visible() && _vertical_thumb.on_ui_input_event(event))
        return true;
    if (_horizontal_thumb.is_visible() && _horizontal_thumb.on_ui_input_event(event))
        return true;
    return false;
}

bool UiScrollContainer::shows_scrollbar(UiScrollAxis axis) const noexcept
{
    if (_scrollbar_visibility == UiScrollBarVisibility::Hidden)
        return false;

    const UiScrollAxis resolved = _scroll_state.resolved_axis();
    const bool axis_supported = is_horizontal_axis(axis)
        ? (resolved == UiScrollAxis::Horizontal || resolved == UiScrollAxis::Both)
        : (resolved == UiScrollAxis::Vertical || resolved == UiScrollAxis::Both);
    if (!axis_supported)
        return false;

    if (_scrollbar_visibility == UiScrollBarVisibility::Always)
        return true;
    return is_horizontal_axis(axis) ? _scroll_state.can_scroll_horizontal() : _scroll_state.can_scroll_vertical();
}

elysia::core::Rect UiScrollContainer::scrollbar_track_rect(UiScrollAxis axis) const noexcept
{
    if (!shows_scrollbar(axis))
        return elysia::core::Rect::zero();

    const elysia::core::Rect viewport = content_rect();
    const float thickness = std::max(1.0f,_scrollbar_style.thickness);
    const float margin = clamp_non_negative(_scrollbar_style.margin);
    const bool horizontal_visible = shows_scrollbar(UiScrollAxis::Horizontal);
    const bool vertical_visible = shows_scrollbar(UiScrollAxis::Vertical);

    if (is_horizontal_axis(axis))
    {
        const float x = viewport.x() + margin;
        const float y = viewport.bottom() - margin - thickness;
        const float width = std::max(0.0f,viewport.width() - margin * 2.0f - (vertical_visible ? thickness + margin : 0.0f));
        return elysia::core::Rect(x,y,width,thickness);
    }

    const float x = viewport.right() - margin - thickness;
    const float y = viewport.y() + margin;
    const float height = std::max(0.0f,viewport.height() - margin * 2.0f - (horizontal_visible ? thickness + margin : 0.0f));
    return elysia::core::Rect(x,y,thickness,height);
}

elysia::core::Rect UiScrollContainer::scrollbar_thumb_rect(UiScrollAxis axis,const elysia::core::Rect& track_rect) const noexcept
{
    if (track_rect.is_empty())
        return elysia::core::Rect::zero();

    const float min_thumb_length = clamp_non_negative(_scrollbar_style.min_thumb_length);
    const elysia::core::Vector2 viewport = _scroll_state.viewport_size();
    const elysia::core::Vector2 content = _scroll_state.effective_content_size();

    if (is_horizontal_axis(axis))
    {
        const float track_length = track_rect.width();
        const float visible_ratio = content.x > ScrollbarEpsilon ? clamp01(viewport.x / std::max(content.x,viewport.x)) : 1.0f;
        const float min_length = std::min(min_thumb_length,track_length);
        const float thumb_length = _scroll_state.can_scroll_horizontal()
            ? std::clamp(track_length * visible_ratio,min_length,track_length)
            : track_length;
        const float span = std::max(0.0f,track_length - thumb_length);
        const float x = track_rect.x() + span * _scroll_state.horizontal_ratio();
        return elysia::core::Rect(x,track_rect.y(),thumb_length,track_rect.height());
    }

    const float track_length = track_rect.height();
    const float visible_ratio = content.y > ScrollbarEpsilon ? clamp01(viewport.y / std::max(content.y,viewport.y)) : 1.0f;
    const float min_length = std::min(min_thumb_length,track_length);
    const float thumb_length = _scroll_state.can_scroll_vertical()
        ? std::clamp(track_length * visible_ratio,min_length,track_length)
        : track_length;
    const float span = std::max(0.0f,track_length - thumb_length);
    const float y = track_rect.y() + span * _scroll_state.vertical_ratio();
    return elysia::core::Rect(track_rect.x(),y,track_rect.width(),thumb_length);
}

elysia::core::Color UiScrollContainer::current_track_color(const UiDragHandle& thumb) const noexcept
{
    if (!thumb.is_enabled())
        return _scrollbar_style.track_disabled_color;
    if (thumb.is_dragging())
        return _scrollbar_style.track_dragging_color;
    if (thumb.is_focused())
        return _scrollbar_style.track_focused_color;
    return _scrollbar_style.track_idle_color;
}

void UiScrollContainer::initialize_scrollbar_handles()
{
    _horizontal_thumb.reset();
    _vertical_thumb.reset();

    _horizontal_thumb.set_use_theme(false);
    _vertical_thumb.set_use_theme(false);

    _horizontal_thumb.set_on_dragged([this](const elysia::core::Vector2&)
    {
        update_horizontal_offset_from_thumb();
    });
    _vertical_thumb.set_on_dragged([this](const elysia::core::Vector2&)
    {
        update_vertical_offset_from_thumb();
    });

    sync_scrollbar_handles();
}

void UiScrollContainer::sync_scroll_state_to_viewport() noexcept
{
    _scroll_state.set_viewport_size(content_rect().size());
}

void UiScrollContainer::sync_scrollbar_handles() noexcept
{
    const elysia::core::Rect horizontal_track = scrollbar_track_rect(UiScrollAxis::Horizontal);
    const elysia::core::Rect vertical_track = scrollbar_track_rect(UiScrollAxis::Vertical);
    configure_scrollbar_thumb(_horizontal_thumb,UiScrollAxis::Horizontal,horizontal_track,scrollbar_thumb_rect(UiScrollAxis::Horizontal,horizontal_track));
    configure_scrollbar_thumb(_vertical_thumb,UiScrollAxis::Vertical,vertical_track,scrollbar_thumb_rect(UiScrollAxis::Vertical,vertical_track));
}

void UiScrollContainer::configure_scrollbar_thumb(
    UiDragHandle& thumb,
    UiScrollAxis axis,
    const elysia::core::Rect& track_rect,
    const elysia::core::Rect& thumb_rect
) noexcept
{
    const bool visible = shows_scrollbar(axis) && !track_rect.is_empty() && !thumb_rect.is_empty();
    if (!visible)
    {
        thumb.cancel_drag();
        thumb.set_visible(false);
        thumb.set_active(false);
        thumb.set_enabled(false);
        thumb.clear_drag_bounds();
        thumb.set_screen_rect(elysia::core::Rect::zero());
        return;
    }

    UiDragHandleConfig config{};
    config.size = thumb_rect.size();
    config.axis = is_horizontal_axis(axis) ? UiDragAxis::Horizontal : UiDragAxis::Vertical;
    config.drag_bounds = track_rect;
    config.draw_background = true;
    config.draw_border = false;
    config.idle_color = _scrollbar_style.thumb_idle_color;
    config.focused_color = _scrollbar_style.thumb_focused_color;
    config.dragging_color = _scrollbar_style.thumb_dragging_color;
    config.disabled_background_color = _scrollbar_style.thumb_disabled_color;
    config.border_color = _scrollbar_style.thumb_idle_color;
    config.disabled_border_color = _scrollbar_style.thumb_disabled_color;

    thumb.set_drag_handle_config(config);
    thumb.set_drag_axis(config.axis);
    thumb.set_drag_bounds(track_rect);
    thumb.set_screen_rect(thumb_rect);
    thumb.set_visible(true);
    thumb.set_active(true);
    thumb.set_enabled(true);
    thumb.set_opacity(opacity());
}

void UiScrollContainer::update_horizontal_offset_from_thumb() noexcept
{
    const elysia::core::Rect track_rect = scrollbar_track_rect(UiScrollAxis::Horizontal);
    const elysia::core::Rect thumb_rect = _horizontal_thumb.screen_rect();
    if (track_rect.is_empty() || thumb_rect.is_empty())
        return;

    const float span = std::max(0.0f,track_rect.width() - thumb_rect.width());
    const float ratio = span > ScrollbarEpsilon ? clamp01((thumb_rect.x() - track_rect.x()) / span) : 0.0f;
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.set_horizontal_ratio(ratio);
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::update_vertical_offset_from_thumb() noexcept
{
    const elysia::core::Rect track_rect = scrollbar_track_rect(UiScrollAxis::Vertical);
    const elysia::core::Rect thumb_rect = _vertical_thumb.screen_rect();
    if (track_rect.is_empty() || thumb_rect.is_empty())
        return;

    const float span = std::max(0.0f,track_rect.height() - thumb_rect.height());
    const float ratio = span > ScrollbarEpsilon ? clamp01((thumb_rect.y() - track_rect.y()) / span) : 0.0f;
    const elysia::core::Vector2 before = _scroll_state.offset();
    _scroll_state.set_vertical_ratio(ratio);
    if (!_scroll_state.offset().nearly_equals(before))
        mark_layout_dirty();
}

void UiScrollContainer::reset_scroll_offset() noexcept
{
    _scroll_state.set_offset(elysia::core::Vector2::zero());
    mark_layout_dirty();
}

void UiScrollContainer::ensure_visible_focused_target() noexcept
{
    const UiControl* target = focused_target();
    if (target)
        ensure_visible(target->screen_rect());
}

void UiScrollContainer::submit_scrollbar_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    const elysia::core::Rect viewport = content_rect();

    const auto submit_thumb = [this,&out_commands,&viewport](const UiDragHandle& thumb)
    {
        if (!thumb.is_visible())
            return;
        auto& mutable_thumb = const_cast<UiDragHandle&>(thumb);
        mutable_thumb.set_opacity(opacity());
        const std::size_t begin = out_commands.size();
        thumb.submit_ui_render_commands(out_commands);
        apply_clip_to_range(out_commands,begin,viewport);
    };

    if (_horizontal_thumb.is_visible())
    {
        if (_scrollbar_style.draw_track)
        {
            elysia::core::UiRenderCommand track = elysia::core::make_ui_fill_rect_command(
                scrollbar_track_rect(UiScrollAxis::Horizontal),
                current_track_color(_horizontal_thumb)
            );
            apply_opacity(track);
            out_commands.push_back(track);
        }
        submit_thumb(_horizontal_thumb);
    }

    if (_vertical_thumb.is_visible())
    {
        if (_scrollbar_style.draw_track)
        {
            elysia::core::UiRenderCommand track = elysia::core::make_ui_fill_rect_command(
                scrollbar_track_rect(UiScrollAxis::Vertical),
                current_track_color(_vertical_thumb)
            );
            apply_opacity(track);
            out_commands.push_back(track);
        }
        submit_thumb(_vertical_thumb);
    }
}
}
