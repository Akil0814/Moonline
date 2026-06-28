#include "ui_list_container.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
[[nodiscard]] float clamp_non_negative(float value) noexcept
{
    return std::max(0.0f,value);
}

[[nodiscard]] elysia::core::Vector2 clamp_size(const elysia::core::Vector2& size) noexcept
{
    return elysia::core::Vector2(clamp_non_negative(size.x),clamp_non_negative(size.y));
}

[[nodiscard]] elysia::core::Rect align_rect_in_bounds(
    const elysia::core::Rect& bounds,
    const elysia::core::Vector2& size,
    UiLayoutAnchor anchor,
    const UiLayoutMargin& margin
) noexcept
{
    const elysia::core::Vector2 clamped_size(
        std::min(clamp_non_negative(size.x),bounds.width()),
        std::min(clamp_non_negative(size.y),bounds.height())
    );
    elysia::core::Rect rect = elysia::core::Rect::from_center(bounds.center(),clamped_size);

    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
        rect.set_position(elysia::core::Vector2(bounds.left() + margin.left,bounds.top() + margin.top));
        break;
    case UiLayoutAnchor::TopCenter:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f + margin.left - margin.right,bounds.top() + margin.top));
        break;
    case UiLayoutAnchor::TopRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x - margin.right,bounds.top() + margin.top));
        break;
    case UiLayoutAnchor::CenterLeft:
        rect.set_position(elysia::core::Vector2(bounds.left() + margin.left,bounds.center().y - clamped_size.y * 0.5f + margin.top - margin.bottom));
        break;
    case UiLayoutAnchor::Center:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f + margin.left - margin.right,bounds.center().y - clamped_size.y * 0.5f + margin.top - margin.bottom));
        break;
    case UiLayoutAnchor::CenterRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x - margin.right,bounds.center().y - clamped_size.y * 0.5f + margin.top - margin.bottom));
        break;
    case UiLayoutAnchor::BottomLeft:
        rect.set_position(elysia::core::Vector2(bounds.left() + margin.left,bounds.bottom() - clamped_size.y - margin.bottom));
        break;
    case UiLayoutAnchor::BottomCenter:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f + margin.left - margin.right,bounds.bottom() - clamped_size.y - margin.bottom));
        break;
    case UiLayoutAnchor::BottomRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x - margin.right,bounds.bottom() - clamped_size.y - margin.bottom));
        break;
    }

    return rect;
}
}

UiListContainer::UiListContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiContainer(rect,order) {}

UiListContainer::UiListContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiContainer(position,size,order) {}

UiListContainer::UiListContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiContainer(center,size,from_center,order) {}

void UiListContainer::reset() noexcept
{
    UiContainer::reset();
    _direction = UiListDirection::Vertical;
    _item_spacing = 0.0f;
}

void UiListContainer::set_direction(UiListDirection direction) noexcept
{
    _direction = direction;
    mark_layout_dirty();
}

UiListDirection UiListContainer::direction() const noexcept
{
    return _direction;
}

void UiListContainer::set_item_spacing(float item_spacing) noexcept
{
    _item_spacing = clamp_non_negative(item_spacing);
    mark_layout_dirty();
}

float UiListContainer::item_spacing() const noexcept
{
    return _item_spacing;
}

void UiListContainer::rebuild_layout()
{
    std::vector<ChildEntry>& child_entries = children();
    if (child_entries.empty())
        return;

    const elysia::core::Rect bounds = content_rect();
    float cursor = 0.0f;

    for (ChildEntry& entry : child_entries)
    {
        if (!entry.element)
            continue;

        elysia::core::Vector2 child_size = entry.layout._use_size_override ? clamp_size(entry.layout._size_override) : clamp_size(entry.element->size());
        if (_direction == UiListDirection::Vertical)
        {
            if (entry.layout._fill_cross_axis)
                child_size.x = bounds.width();
            const elysia::core::Rect slot_rect(bounds.x(),bounds.y() + cursor,bounds.width(),child_size.y);
            entry.element->set_screen_rect(align_rect_in_bounds(slot_rect,child_size,entry.layout._anchor,entry.layout._margin));
            cursor += child_size.y + _item_spacing;
        }
        else
        {
            if (entry.layout._fill_cross_axis)
                child_size.y = bounds.height();
            const elysia::core::Rect slot_rect(bounds.x() + cursor,bounds.y(),child_size.x,bounds.height());
            entry.element->set_screen_rect(align_rect_in_bounds(slot_rect,child_size,entry.layout._anchor,entry.layout._margin));
            cursor += child_size.x + _item_spacing;
        }
    }
}
}
