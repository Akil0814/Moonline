#include "digit_cache.h"

#include "../core/render/sdl_convert.h"
#include "../resources/resource_manager.h"

#include <SDL_ttf.h>

#include <utility>

namespace elysia::number
{
namespace
{
constexpr const char* kDefaultFontKeyPrefix = "ui.latin.";
}

SDL_Texture* DigitTextureSource::glyph_texture(char ch) const noexcept
{
    if (ch >= '0' && ch <= '9')
    {
        return digits[static_cast<std::size_t>(ch - '0')];
    }

    switch (ch)
    {
    case '-':
        return minus;

    case '.':
        return dot;

    case '/':
        return slash;

    case '%':
        return percent;

    default:
        return nullptr;
    }
}

void DigitCache::set_texture_source(const DigitTextureSource& source)
{
    clear_font_cache();
    _font_source = DigitFontSource{};
    _texture_source = source;
    _source_mode = DigitSourceMode::TextureSet;
}

void DigitCache::set_font_source(const DigitFontSource& source)
{
    clear_font_cache();
    _texture_source = DigitTextureSource{};
    _font_source = source;
    _source_mode = DigitSourceMode::Font;
}

DigitSourceMode DigitCache::source_mode() const noexcept
{
    return _source_mode;
}

SDL_Texture* DigitCache::get_glyph(char ch)
{
    if (!is_supported_glyph(ch))
    {
        return nullptr;
    }

    switch (_source_mode)
    {
    case DigitSourceMode::TextureSet:
        return get_texture_source_glyph(ch);

    case DigitSourceMode::Font:
        return get_or_create_font_glyph(ch);

    case DigitSourceMode::None:
    default:
        return nullptr;
    }
}

bool DigitCache::supports(char ch) const
{
    if (!is_supported_glyph(ch))
    {
        return false;
    }

    switch (_source_mode)
    {
    case DigitSourceMode::TextureSet:
        return get_texture_source_glyph(ch) != nullptr;

    case DigitSourceMode::Font:
        return _font_source.style.point_size > 0;

    case DigitSourceMode::None:
    default:
        return false;
    }
}

void DigitCache::clear_font_cache()
{
    _font_cache.clear();
}

void DigitCache::reset()
{
    clear_font_cache();
    _texture_source = DigitTextureSource{};
    _font_source = DigitFontSource{};
    _source_mode = DigitSourceMode::None;
}

void DigitCache::SdlTextureDeleter::operator()(SDL_Texture* texture) const
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }
}

bool DigitCache::is_supported_glyph(char ch) noexcept
{
    return (ch >= '0' && ch <= '9')
        || ch == '-'
        || ch == '.'
        || ch == '/'
        || ch == '%';
}

SDL_Texture* DigitCache::get_texture_source_glyph(char ch) const noexcept
{
    return _texture_source.glyph_texture(ch);
}

std::string DigitCache::resolve_font_key(char ch) const
{
    if (_font_source.font_key_resolver)
    {
        return _font_source.font_key_resolver(ch, _font_source.style);
    }

    if (!_font_source.font_key.empty())
    {
        return _font_source.font_key;
    }

    return std::string(kDefaultFontKeyPrefix)
        + std::to_string(_font_source.style.point_size);
}

SDL_Texture* DigitCache::get_or_create_font_glyph(char ch)
{
    const auto found = _font_cache.find(ch);
    if (found != _font_cache.end())
    {
        return found->second.get();
    }

    if (!_font_source.renderer || _font_source.style.point_size <= 0)
    {
        return nullptr;
    }

    const std::string font_key = resolve_font_key(ch);
    if (font_key.empty())
    {
        return nullptr;
    }

    TTF_Font* font = elysia::resources::ResourceManager::instance()->find_font(font_key);
    if (!font)
    {
        return nullptr;
    }

    const SDL_Color text_color = elysia::core::to_sdl_color(_font_source.style.color);
    const char glyph_text[2] = { ch, '\0' };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, glyph_text, text_color);
    if (!surface)
    {
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_font_source.renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture)
    {
        return nullptr;
    }

    OwnedTexturePtr owned_texture(texture);
    SDL_Texture* raw_texture = owned_texture.get();
    _font_cache.emplace(ch, std::move(owned_texture));
    return raw_texture;
}

}
