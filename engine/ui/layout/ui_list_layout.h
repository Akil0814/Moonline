#pragma once

#include "ui_layout_types.h"
#include "ui_layout_geometry.h"
#include "../core/ui_child_host.h"

namespace elysia::ui::layout
{
struct UiListLayoutConfig
{
    UiLayoutDirection direction = UiLayoutDirection::Vertical;
    float item_spacing = 0.0f;
};

void layout_list_children(
    std::vector<UiChildHost::ChildEntry>& children,
    const elysia::core::Rect& bounds,
    const UiListLayoutConfig& config
) noexcept;
}
