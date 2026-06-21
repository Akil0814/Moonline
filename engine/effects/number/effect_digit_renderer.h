#pragma once

#include "../../core/geometry/rect.h"
#include "../../core/geometry/vector2.h"
#include "../../core/render/render_command.h"
#include "../../number/digit_cache.h"

#include <optional>
#include <string_view>
#include <vector>

namespace elysia::effects
{
struct EffectDigitRenderRequest
{
    std::string_view text;
    std::optional<elysia::core::Rect> world_rect;
    std::optional<elysia::core::Vector2> world_position;
    std::optional<elysia::core::Vector2> glyph_size;
    elysia::number::DigitAlignment alignment = elysia::number::DigitAlignment::Left;
    float spacing = 0.0f;
    std::optional<float> fixed_glyph_advance;
    std::optional<float> target_height;
    float uniform_scale = 1.0f;
    Uint8 alpha = 255;
    double rotation_degrees = 0.0;
    elysia::core::Vector2 rotation_origin = elysia::core::Vector2(0.5f, 0.5f);
    elysia::core::SpriteFlip flip = elysia::core::SpriteFlip::None;
};

class EffectDigitRenderer
{
public:
    explicit EffectDigitRenderer(elysia::number::DigitCache* digit_cache = nullptr) noexcept;

    void set_digit_cache(elysia::number::DigitCache* digit_cache) noexcept;
    [[nodiscard]] elysia::number::DigitCache* digit_cache() const noexcept;

    void append_render_commands(
        const EffectDigitRenderRequest& request,
        std::vector<elysia::core::RenderCommand>& out_commands
    ) const;

private:
    elysia::number::DigitCache* _digit_cache = nullptr;
};

}
