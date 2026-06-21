#pragma once

#include "../core/render/color.h"

#include <SDL.h>

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace elysia::number
{
enum class DigitSourceMode
{
    None,
    TextureSet,
    Font
};

enum class DigitAlignment
{
    Left,
    Center,
    Right
};

struct DigitTextureSource
{
    std::array<SDL_Texture*, 10> digits{};
    SDL_Texture* minus = nullptr;
    SDL_Texture* dot = nullptr;
    SDL_Texture* slash = nullptr;
    SDL_Texture* percent = nullptr;

    [[nodiscard]] SDL_Texture* glyph_texture(char ch) const noexcept;
};

struct DigitFontStyle
{
    int point_size = 24;
    elysia::core::Color color{};
};

struct DigitFontSource
{
    SDL_Renderer* renderer = nullptr;
    std::string font_key;
    DigitFontStyle style;
    std::function<std::string(char, const DigitFontStyle&)> font_key_resolver;
};

class DigitCache
{
public:
    void set_texture_source(const DigitTextureSource& source);
    void set_font_source(const DigitFontSource& source);

    [[nodiscard]] DigitSourceMode source_mode() const noexcept;
    [[nodiscard]] SDL_Texture* get_glyph(char ch);
    [[nodiscard]] bool supports(char ch) const;

    void clear_font_cache();
    void reset();

private:
    struct SdlTextureDeleter
    {
        void operator()(SDL_Texture* texture) const;
    };

    using OwnedTexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

    [[nodiscard]] static bool is_supported_glyph(char ch) noexcept;
    [[nodiscard]] SDL_Texture* get_texture_source_glyph(char ch) const noexcept;
    [[nodiscard]] std::string resolve_font_key(char ch) const;
    [[nodiscard]] SDL_Texture* get_or_create_font_glyph(char ch);

private:
    DigitSourceMode _source_mode = DigitSourceMode::None;
    DigitTextureSource _texture_source;
    DigitFontSource _font_source;
    std::unordered_map<char, OwnedTexturePtr> _font_cache;
};

}
