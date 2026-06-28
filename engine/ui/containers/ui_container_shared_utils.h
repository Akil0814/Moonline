#pragma once

#include "../../core/geometry/rect.h"
#include "../../core/render/render_command.h"
#include "ui_container_layout_types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace elysia::ui::container_utils
{
[[nodiscard]] elysia::core::Vector2 clamp_size(const elysia::core::Vector2& size) noexcept;
[[nodiscard]] elysia::core::Rect anchored_rect(
    const elysia::core::Rect& bounds,
    const elysia::core::Vector2& size,
    UiLayoutAnchor anchor,
    const UiLayoutMargin& margin
) noexcept;
[[nodiscard]] elysia::core::Rect padded_content_rect(
    const elysia::core::Rect& rect,
    const UiLayoutPadding& padding
) noexcept;
void apply_opacity_to_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    std::uint8_t opacity
) noexcept;
void apply_clip_to_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    const elysia::core::Rect& clip_rect
);
void finalize_child_command_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    std::uint8_t opacity,
    bool clip_children,
    const elysia::core::Rect& clip_rect
);
}
