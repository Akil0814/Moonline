#include "ui_list_container.h"

#include "../layout/ui_list_layout.h"

namespace elysia::ui
{
UiListContainer::UiListContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiChildHost(rect,order) {}

UiListContainer::UiListContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiChildHost(position,size,order) {}

UiListContainer::UiListContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiChildHost(center,size,from_center,order) {}

void UiListContainer::reset() noexcept
{
    UiChildHost::reset();
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
    _item_spacing = layout::clamp_non_negative(item_spacing);
    mark_layout_dirty();
}

float UiListContainer::item_spacing() const noexcept
{
    return _item_spacing;
}

void UiListContainer::rebuild_layout()
{
    layout::layout_list_children(children(),content_rect(),layout::UiListLayoutConfig{
        .direction = _direction == UiListDirection::Vertical ? UiLayoutDirection::Vertical : UiLayoutDirection::Horizontal,
        .item_spacing = _item_spacing
    });
}
}
