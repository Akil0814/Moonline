#include "text_button.h"

#include "../../resources/resource_manager.h"

#include <algorithm>

TextButton::TextButton(Vector2 position, Vector2 size, int order)
    : Button(position, size, order)
{
}

void TextButton::on_render(SDL_Renderer* renderer)
{
    refresh_text_texture(renderer);
    Button::on_render(renderer);
}

void TextButton::reset()
{
    Button::reset();
    _text.clear();
    _font_key.clear();
    _font = nullptr;
    _message_texture.reset();
    _text_color = SDL_Color{ 255, 255, 255, 255 };
    _wrap_width = 0;
    _dirty = true;
    set_message_texture(nullptr);
}

void TextButton::set_text(const std::string& text)
{
    _text = text;
    mark_dirty();
}

const std::string& TextButton::text() const
{
    return _text;
}

void TextButton::set_font(TTF_Font* font)
{
    _font = font;
    mark_dirty();
}

TTF_Font* TextButton::font() const
{
    return _font;
}

void TextButton::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    mark_dirty();
}

const std::string& TextButton::font_key() const
{
    return _font_key;
}

void TextButton::set_text_color(SDL_Color color)
{
    _text_color = color;
    mark_dirty();
}

SDL_Color TextButton::text_color() const
{
    return _text_color;
}

void TextButton::set_wrap_width(int wrap_width)
{
    _wrap_width = std::max(0, wrap_width);
    mark_dirty();
}

int TextButton::wrap_width() const
{
    return _wrap_width;
}

void TextButton::mark_dirty()
{
    _dirty = true;
}

void TextButton::refresh_text_texture(SDL_Renderer* renderer)
{
    if (!_dirty || !renderer)
    {
        return;
    }

    _dirty = false;
    _message_texture.reset();
    set_message_texture(nullptr);

    if (_text.empty())
    {
        return;
    }

    TTF_Font* resolved_font = resolve_font();
    if (!resolved_font)
    {
        return;
    }

    SDL_Surface* text_surface = nullptr;
    if (_wrap_width > 0)
    {
        text_surface = TTF_RenderUTF8_Blended_Wrapped(
            resolved_font,
            _text.c_str(),
            _text_color,
            static_cast<Uint32>(_wrap_width)
        );
    }
    else
    {
        text_surface = TTF_RenderUTF8_Blended(
            resolved_font,
            _text.c_str(),
            _text_color
        );
    }

    if (!text_surface)
    {
        return;
    }

    SDL_Texture* raw_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (!raw_texture)
    {
        return;
    }

    _message_texture.reset(raw_texture);
    SDL_SetTextureBlendMode(_message_texture.get(), SDL_BLENDMODE_BLEND);
    set_message_texture(_message_texture.get());
}

TTF_Font* TextButton::resolve_font() const
{
    if (_font)
    {
        return _font;
    }

    if (_font_key.empty())
    {
        return nullptr;
    }

    return ResourceManager::instance()->find_font(_font_key);
}
