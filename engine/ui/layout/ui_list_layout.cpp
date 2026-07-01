#include "ui_list_layout.h"

namespace elysia::ui::layout
{
namespace
{
[[nodiscard]] UiLayoutAnchor list_anchor(UiLayoutDirection direction,UiLayoutAlign align) noexcept
{
    if (direction == UiLayoutDirection::Vertical)
    {
        switch (align)
        {
        case UiLayoutAlign::Start:
            return UiLayoutAnchor::TopLeft;
        case UiLayoutAlign::End:
            return UiLayoutAnchor::TopRight;
        case UiLayoutAlign::Center:
        default:
            return UiLayoutAnchor::TopCenter;
        }
    }

    switch (align)
    {
    case UiLayoutAlign::Start:
        return UiLayoutAnchor::TopLeft;
    case UiLayoutAlign::End:
        return UiLayoutAnchor::BottomLeft;
    case UiLayoutAlign::Center:
    default:
        return UiLayoutAnchor::CenterLeft;
    }
}
}

void layout_list_children(
    std::vector<UiChildHost::ChildEntry>& children,
    const elysia::core::Rect& bounds,
    const UiListLayoutConfig& config
) noexcept
{
    float cursor = 0.0f;
    const float item_spacing = clamp_non_negative(config.item_spacing);

    for (UiChildHost::ChildEntry& child : children)
    {
        if (!child.element)
            continue;

        elysia::core::Vector2 child_size = child.layout._use_size_override
            ? clamp_size(child.layout._size_override)
            : clamp_size(child.element->size());
        const UiLayoutAlign cross_align = child.layout._use_custom_cross_align ? child.layout._cross_align : config.cross_align;
        const UiLayoutAnchor anchor = list_anchor(config.direction,cross_align);

        if (config.direction == UiLayoutDirection::Vertical)
        {
            if (child.layout._fill_cross_axis)
                child_size.x = bounds.width();
            const elysia::core::Rect slot_rect(bounds.x(),bounds.y() + cursor,bounds.width(),child_size.y);
            child.element->set_screen_rect(aligned_rect_in_bounds(slot_rect,child_size,anchor,child.layout._margin));
            cursor += child_size.y + item_spacing;
        }
        else
        {
            if (child.layout._fill_cross_axis)
                child_size.y = bounds.height();
            const elysia::core::Rect slot_rect(bounds.x() + cursor,bounds.y(),child_size.x,bounds.height());
            child.element->set_screen_rect(aligned_rect_in_bounds(slot_rect,child_size,anchor,child.layout._margin));
            cursor += child_size.x + item_spacing;
        }
    }
}
}
