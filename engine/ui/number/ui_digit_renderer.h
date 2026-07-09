#pragma once

#include "../../core/geometry/rect.h"
#include "../../core/geometry/vector2.h"
#include "../../core/render/render_command.h"
#include "../core/ui_text_align.h"
#include "../../number/digit_cache.h"

#include <optional>
#include <string_view>
#include <vector>

namespace elysia::ui
{
// Describes how a numeric glyph string should be positioned and scaled when rendered.
struct UiDigitRenderRequest
{
    std::string_view text;
    std::optional<elysia::core::Rect> target_rect;
    std::optional<elysia::core::Vector2> anchor_position;
    elysia::number::DigitAlignment alignment = elysia::number::DigitAlignment::Left;
    TextVerticalAlign vertical_align = TextVerticalAlign::Center;
    float spacing = 0.0f;
    std::optional<float> fixed_glyph_advance;
    std::optional<float> target_height;
    float uniform_scale = 1.0f;
    Uint8 alpha = 255;
};

class UiDigitRenderer
{
public:
    explicit UiDigitRenderer(elysia::number::DigitCache* digit_cache = nullptr) noexcept;

    void set_digit_cache(elysia::number::DigitCache* digit_cache) noexcept;
    [[nodiscard]] elysia::number::DigitCache* digit_cache() const noexcept;

    // Appends digit draw commands without taking ownership of the destination buffer.
    void append_render_commands(
        const UiDigitRenderRequest& request,
        std::vector<elysia::core::UiRenderCommand>& out_commands
    ) const;

private:
    elysia::number::DigitCache* _digit_cache = nullptr;
};

}
