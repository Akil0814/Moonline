#include "ui_grid_container.h"

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

UiGridContainer::UiGridContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiContainer(rect,order) {}

UiGridContainer::UiGridContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiContainer(position,size,order) {}

UiGridContainer::UiGridContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiContainer(center,size,from_center,order) {}

void UiGridContainer::reset() noexcept
{
    UiContainer::reset();
    _column_count = 1;
    _cell_spacing = elysia::core::Vector2(0.0f,0.0f);
    _fill_by_row = true;
}

void UiGridContainer::set_column_count(int column_count) noexcept
{
    _column_count = std::max(1,column_count);
    mark_layout_dirty();
}

int UiGridContainer::column_count() const noexcept
{
    return _column_count;
}

void UiGridContainer::set_cell_spacing(const elysia::core::Vector2& spacing) noexcept
{
    _cell_spacing = clamp_size(spacing);
    mark_layout_dirty();
}

elysia::core::Vector2 UiGridContainer::cell_spacing() const noexcept
{
    return _cell_spacing;
}

void UiGridContainer::set_fill_by_row(bool fill_by_row) noexcept
{
    _fill_by_row = fill_by_row;
    mark_layout_dirty();
}

bool UiGridContainer::fills_by_row() const noexcept
{
    return _fill_by_row;
}

void UiGridContainer::rebuild_layout()
{
    std::vector<ChildEntry>& child_entries = children();
    if (child_entries.empty())
        return;

    const elysia::core::Rect bounds = content_rect();
    const int columns = std::max(1,_column_count);
    const int count = static_cast<int>(child_entries.size());
    const int rows = std::max(1,(count + columns - 1) / columns);
    const float total_spacing_x = _cell_spacing.x * static_cast<float>(std::max(0,columns - 1));
    const float total_spacing_y = _cell_spacing.y * static_cast<float>(std::max(0,rows - 1));
    const float cell_width = columns > 0 ? std::max(0.0f,(bounds.width() - total_spacing_x) / static_cast<float>(columns)) : 0.0f;
    const float cell_height = rows > 0 ? std::max(0.0f,(bounds.height() - total_spacing_y) / static_cast<float>(rows)) : 0.0f;

    for (int index = 0; index < count; ++index)
    {
        ChildEntry& entry = child_entries[static_cast<std::size_t>(index)];
        if (!entry.element)
            continue;

        const int row = _fill_by_row ? index / columns : index % rows;
        const int column = _fill_by_row ? index % columns : index / rows;
        const elysia::core::Rect cell_rect(
            bounds.x() + static_cast<float>(column) * (cell_width + _cell_spacing.x),
            bounds.y() + static_cast<float>(row) * (cell_height + _cell_spacing.y),
            cell_width,
            cell_height
        );
        const elysia::core::Vector2 child_size = entry.layout._use_size_override
            ? clamp_size(entry.layout._size_override)
            : cell_rect.size();
        entry.element->set_screen_rect(align_rect_in_bounds(cell_rect,child_size,entry.layout._anchor,entry.layout._margin));
    }
}
}
