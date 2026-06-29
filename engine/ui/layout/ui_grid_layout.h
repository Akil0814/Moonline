#pragma once

#include "ui_layout_geometry.h"
#include "../core/ui_child_host.h"

namespace elysia::ui::layout
{
struct UiGridLayoutConfig
{
    int column_count = 1;
    elysia::core::Vector2 cell_spacing{ 0.0f,0.0f };
    bool fill_by_row = true;
};

void layout_grid_children(
    std::vector<UiChildHost::ChildEntry>& children,
    const elysia::core::Rect& bounds,
    const UiGridLayoutConfig& config
) noexcept;
}
