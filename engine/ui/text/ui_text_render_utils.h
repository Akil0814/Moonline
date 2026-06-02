#pragma once

#include "../../resources/texture/texture_loader.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

struct UiTextTextureResult
{
    TexturePtr _texture;
    int _width = 0;
    int _height = 0;
};

TTF_Font* ui_resolve_font(TTF_Font* font, const std::string& font_key);

UiTextTextureResult ui_render_text_texture(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const std::string& text,
    SDL_Color color,
    int wrap_width = 0
);
