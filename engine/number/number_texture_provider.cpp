#include "number_texture_provider.h"

#include "../localization/localization_manager.h"

#include <SDL.h>

#include <algorithm>

namespace elysia::number
{
std::vector<NumberTextureGlyph> NumberTextureProvider::get_texture_set(
    std::string_view text,
    int point_size,
    elysia::core::Color color
) const
{
    ensure_font_source(point_size, color);

    std::vector<NumberTextureGlyph> glyphs;
    glyphs.reserve(text.size());

    for (const char ch : text)
    {
        SDL_Texture* texture = _digit_cache.get_glyph(ch);
        if (!texture)
        {
            continue;
        }

        int texture_width = 0;
        int texture_height = 0;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height) != 0
            || texture_width <= 0
            || texture_height <= 0)
        {
            continue;
        }

        NumberTextureGlyph glyph;
        glyph.texture = texture;
        glyph.texture_width = texture_width;
        glyph.texture_height = texture_height;
        glyphs.push_back(glyph);
    }

    return glyphs;
}

void NumberTextureProvider::reset() noexcept
{
    _digit_cache.reset();
    _configured_renderer = nullptr;
    _configured_point_size = 0;
    _configured_text_color = {};
    _has_configured_font_source = false;
}

void NumberTextureProvider::ensure_font_source(int point_size, elysia::core::Color color) const
{
    elysia::localization::LocalizationManager* localization_manager = elysia::localization::LocalizationManager::instance();
    if (!localization_manager)
    {
        return;
    }

    SDL_Renderer* renderer = localization_manager->renderer();
    if (!renderer)
    {
        return;
    }

    const int clamped_point_size = std::max(0, point_size);
    if (_has_configured_font_source
        && renderer == _configured_renderer
        && clamped_point_size == _configured_point_size
        && color == _configured_text_color)
    {
        return;
    }

    DigitFontSource font_source;
    font_source.renderer = renderer;
    font_source.style.point_size = clamped_point_size;
    font_source.style.color = color;
    _digit_cache.set_font_source(font_source);

    _configured_renderer = renderer;
    _configured_point_size = clamped_point_size;
    _configured_text_color = color;
    _has_configured_font_source = true;
}
}
