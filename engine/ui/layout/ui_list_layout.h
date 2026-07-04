#pragma once

#include "ui_layout_types.h"
#include "ui_layout_geometry.h"
#include "../core/ui_child_host.h"

namespace elysia::ui::layout
{
struct UiListLayoutConfig
{
    UiLayoutDirection direction = UiLayoutDirection::Vertical;
    UiLayoutAlign cross_align = UiLayoutAlign::Center;
    float item_spacing = 16.0f;
};

void layout_list_children(
    std::vector<UiChildHost::ChildEntry>& children,
    const elysia::core::Rect& bounds,
    const UiListLayoutConfig& config
) noexcept;

[[nodiscard]] elysia::core::Vector2 intrinsic_list_extent(
    const std::vector<UiChildHost::ChildEntry>& children,
    const UiLayoutPadding& padding,
    const UiListLayoutConfig& config
) noexcept;
}
