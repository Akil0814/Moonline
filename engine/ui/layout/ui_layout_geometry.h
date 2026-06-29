#pragma once

#include "ui_layout_types.h"
#include "../../core/geometry/rect.h"

namespace elysia::ui::layout
{
[[nodiscard]] float clamp_non_negative(float value) noexcept;
[[nodiscard]] elysia::core::Vector2 clamp_size(const elysia::core::Vector2& size) noexcept;
[[nodiscard]] elysia::core::Rect padded_content_rect(const elysia::core::Rect& rect,const UiLayoutPadding& padding) noexcept;
[[nodiscard]] elysia::core::Rect anchored_rect(
    const elysia::core::Rect& bounds,
    const elysia::core::Vector2& size,
    UiLayoutAnchor anchor,
    const UiLayoutMargin& margin
) noexcept;
[[nodiscard]] elysia::core::Rect aligned_rect_in_bounds(
    const elysia::core::Rect& bounds,
    const elysia::core::Vector2& size,
    UiLayoutAnchor anchor,
    const UiLayoutMargin& margin
) noexcept;
}
