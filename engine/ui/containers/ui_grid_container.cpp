#include "ui_grid_container.h"

#include "../layout/ui_grid_layout.h"

#include <algorithm>

namespace elysia::ui
{
UiGridContainer::UiGridContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiChildHost(rect,order) {}

UiGridContainer::UiGridContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiChildHost(position,size,order) {}

UiGridContainer::UiGridContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiChildHost(center,size,from_center,order) {}

void UiGridContainer::reset() noexcept
{
    UiChildHost::reset();
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
    _cell_spacing = layout::clamp_size(spacing);
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
    layout::layout_grid_children(children(),content_rect(),layout::UiGridLayoutConfig{
        .column_count = _column_count,
        .cell_spacing = _cell_spacing,
        .fill_by_row = _fill_by_row
    });
}
}
