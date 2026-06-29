#pragma once

#include "../../core/render/render_command.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace elysia::ui::render_command_range_utils
{
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
