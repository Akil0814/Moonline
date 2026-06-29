#include "ui_scroll_container.h"

#include "../layout/ui_layout_geometry.h"

#include <algorithm>

namespace elysia::ui
{
UiScrollContainer::UiScrollContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiChildHost(rect,order) {}

UiScrollContainer::UiScrollContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiChildHost(position,size,order) {}

UiScrollContainer::UiScrollContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiChildHost(center,size,from_center,order) {}

void UiScrollContainer::reset() noexcept
{
    UiChildHost::reset();
    UiChildHost::set_clip_children(true);
    _content_size = elysia::core::Vector2::zero();
    _scroll_offset = elysia::core::Vector2::zero();
    _scroll_step = elysia::core::Vector2(24.0f,24.0f);
}

bool UiScrollContainer::on_ui_input_event(const UiInputEvent& event)
{
    UiChildHost::set_clip_children(true);
    update_layout_if_dirty();

    if (event.type == UiInputEventType::MouseWheel)
    {
        const elysia::core::Rect viewport = content_rect();
        const elysia::core::Vector2 point(static_cast<float>(event.mouse_x),static_cast<float>(event.mouse_y));
        if (viewport.contains(point))
        {
            const elysia::core::Vector2 before = _scroll_offset;
            const float max_x = max_scroll_offset_x();
            const float max_y = max_scroll_offset_y();

            if (event.wheel_x != 0 && max_x > 0.0f)
                set_scroll_offset_x(_scroll_offset.x - static_cast<float>(event.wheel_x) * _scroll_step.x);
            if (event.wheel_y != 0)
            {
                if (max_y > 0.0f)
                    set_scroll_offset_y(_scroll_offset.y - static_cast<float>(event.wheel_y) * _scroll_step.y);
                else if (max_x > 0.0f)
                    set_scroll_offset_x(_scroll_offset.x - static_cast<float>(event.wheel_y) * _scroll_step.x);
            }

            if (!_scroll_offset.nearly_equals(before))
                return true;
        }
    }

    return UiChildHost::on_ui_input_event(event);
}

UiElement* UiScrollContainer::set_content(std::unique_ptr<UiElement> content)
{
    clear_children();
    return add_child(std::move(content));
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
    clear_children();
}

void UiScrollContainer::set_content_size(const elysia::core::Vector2& content_size) noexcept
{
    _content_size = layout::clamp_size(content_size);
    _scroll_offset = clamp_scroll_offset(_scroll_offset);
    mark_layout_dirty();
}

elysia::core::Vector2 UiScrollContainer::content_size() const noexcept
{
    return _content_size;
}

void UiScrollContainer::set_content_width(float content_width) noexcept
{
    _content_size.x = layout::clamp_non_negative(content_width);
    _scroll_offset = clamp_scroll_offset(_scroll_offset);
    mark_layout_dirty();
}

float UiScrollContainer::content_width() const noexcept
{
    return _content_size.x;
}

void UiScrollContainer::set_content_height(float content_height) noexcept
{
    _content_size.y = layout::clamp_non_negative(content_height);
    _scroll_offset = clamp_scroll_offset(_scroll_offset);
    mark_layout_dirty();
}

float UiScrollContainer::content_height() const noexcept
{
    return _content_size.y;
}

void UiScrollContainer::set_scroll_offset(const elysia::core::Vector2& scroll_offset) noexcept
{
    _scroll_offset = clamp_scroll_offset(scroll_offset);
    mark_layout_dirty();
}

void UiScrollContainer::set_scroll_offset(float scroll_offset) noexcept
{
    set_scroll_offset_y(scroll_offset);
}

float UiScrollContainer::scroll_offset() const noexcept
{
    return _scroll_offset.y;
}

void UiScrollContainer::set_scroll_offset_x(float scroll_offset_x) noexcept
{
    set_scroll_offset(elysia::core::Vector2(scroll_offset_x,_scroll_offset.y));
}

float UiScrollContainer::scroll_offset_x() const noexcept
{
    return _scroll_offset.x;
}

void UiScrollContainer::set_scroll_offset_y(float scroll_offset_y) noexcept
{
    set_scroll_offset(elysia::core::Vector2(_scroll_offset.x,scroll_offset_y));
}

float UiScrollContainer::scroll_offset_y() const noexcept
{
    return _scroll_offset.y;
}

void UiScrollContainer::set_scroll_step(const elysia::core::Vector2& scroll_step) noexcept
{
    _scroll_step = layout::clamp_size(scroll_step);
}

void UiScrollContainer::set_scroll_step(float scroll_step) noexcept
{
    set_scroll_step_y(scroll_step);
}

float UiScrollContainer::scroll_step() const noexcept
{
    return _scroll_step.y;
}

void UiScrollContainer::set_scroll_step_x(float scroll_step_x) noexcept
{
    _scroll_step.x = layout::clamp_non_negative(scroll_step_x);
}

float UiScrollContainer::scroll_step_x() const noexcept
{
    return _scroll_step.x;
}

void UiScrollContainer::set_scroll_step_y(float scroll_step_y) noexcept
{
    _scroll_step.y = layout::clamp_non_negative(scroll_step_y);
}

float UiScrollContainer::scroll_step_y() const noexcept
{
    return _scroll_step.y;
}

void UiScrollContainer::scroll_to_left() noexcept
{
    set_scroll_offset_x(0.0f);
}

void UiScrollContainer::scroll_to_right() noexcept
{
    set_scroll_offset_x(max_scroll_offset_x());
}

void UiScrollContainer::scroll_to_top() noexcept
{
    set_scroll_offset_y(0.0f);
}

void UiScrollContainer::scroll_to_bottom() noexcept
{
    set_scroll_offset_y(max_scroll_offset_y());
}

void UiScrollContainer::rebuild_layout()
{
    UiChildHost::set_clip_children(true);
    _scroll_offset = clamp_scroll_offset(_scroll_offset);

    std::vector<ChildEntry>& child_entries = children();
    const elysia::core::Rect viewport = content_rect();
    const elysia::core::Vector2 size = effective_content_size();
    for (std::size_t index = 0; index < child_entries.size(); ++index)
    {
        ChildEntry& entry = child_entries[index];
        if (!entry.element)
            continue;
        if (index == 0)
        {
            entry.element->set_screen_rect(elysia::core::Rect(
                viewport.x() - _scroll_offset.x,
                viewport.y() - _scroll_offset.y,
                size.x,
                size.y
            ));
        }
        else
        {
            entry.element->set_screen_rect(elysia::core::Rect::zero());
        }
    }
}

elysia::core::Vector2 UiScrollContainer::effective_content_size() const noexcept
{
    const elysia::core::Rect viewport = content_rect();
    return elysia::core::Vector2(
        _content_size.x > 0.0f ? _content_size.x : viewport.width(),
        _content_size.y > 0.0f ? _content_size.y : viewport.height()
    );
}

elysia::core::Vector2 UiScrollContainer::clamp_scroll_offset(const elysia::core::Vector2& scroll_offset) const noexcept
{
    return elysia::core::Vector2(
        std::clamp(scroll_offset.x,0.0f,max_scroll_offset_x()),
        std::clamp(scroll_offset.y,0.0f,max_scroll_offset_y())
    );
}

float UiScrollContainer::max_scroll_offset_x() const noexcept
{
    return std::max(0.0f,effective_content_size().x - content_rect().width());
}

float UiScrollContainer::max_scroll_offset_y() const noexcept
{
    return std::max(0.0f,effective_content_size().y - content_rect().height());
}

float UiScrollContainer::max_scroll_offset() const noexcept
{
    return max_scroll_offset_y();
}
}
