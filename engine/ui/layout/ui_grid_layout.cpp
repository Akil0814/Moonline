#include "ui_grid_layout.h"

#include <algorithm>

namespace elysia::ui::layout
{
void layout_grid_children(
    std::vector<UiChildHost::ChildEntry>& children,
    const elysia::core::Rect& bounds,
    const UiGridLayoutConfig& config
) noexcept
{
    if (children.empty())
        return;

    const int columns = std::max(1,config.column_count);
    const int count = static_cast<int>(children.size());
    const int rows = std::max(1,(count + columns - 1) / columns);
    const elysia::core::Vector2 spacing = clamp_size(config.cell_spacing);
    const float total_spacing_x = spacing.x * static_cast<float>(std::max(0,columns - 1));
    const float total_spacing_y = spacing.y * static_cast<float>(std::max(0,rows - 1));
    const float cell_width = std::max(0.0f,(bounds.width() - total_spacing_x) / static_cast<float>(columns));
    const float cell_height = std::max(0.0f,(bounds.height() - total_spacing_y) / static_cast<float>(rows));

    for (int index = 0; index < count; ++index)
    {
        UiChildHost::ChildEntry& child = children[static_cast<std::size_t>(index)];
        if (!child.element)
            continue;

        const int row = config.fill_by_row ? index / columns : index % rows;
        const int column = config.fill_by_row ? index % columns : index / rows;
        const elysia::core::Rect cell_rect(
            bounds.x() + static_cast<float>(column) * (cell_width + spacing.x),
            bounds.y() + static_cast<float>(row) * (cell_height + spacing.y),
            cell_width,
            cell_height
        );
        const elysia::core::Vector2 child_size = child.layout._use_size_override
            ? clamp_size(child.layout._size_override)
            : cell_rect.size();
        child.element->set_screen_rect(aligned_rect_in_bounds(cell_rect,child_size,child.layout._anchor,child.layout._margin));
    }
}
}
