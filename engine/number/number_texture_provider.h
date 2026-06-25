#pragma once

#include "../core/render/color.h"
#include "digit_cache.h"

#include <string_view>
#include <vector>

struct SDL_Texture;

namespace elysia::number
{
struct NumberTextureGlyph
{
    SDL_Texture* texture = nullptr;
    int texture_width = 0;
    int texture_height = 0;
};

class NumberTextureProvider
{
public:
    [[nodiscard]] std::vector<NumberTextureGlyph> get_texture_set(
        std::string_view text,
        int point_size,
        elysia::core::Color color
    ) const;

    void reset() noexcept;

private:
    void ensure_font_source(int point_size, elysia::core::Color color) const;

private:
    mutable DigitCache _digit_cache;
    mutable SDL_Renderer* _configured_renderer = nullptr;
    mutable int _configured_point_size = 0;
    mutable elysia::core::Color _configured_text_color{};
    mutable bool _has_configured_font_source = false;
};
}
