#include "ui_text_render_utils.h"

#include "../../resources/resource_manager.h"

TTF_Font* ui_resolve_font(TTF_Font* font, const std::string& font_key)
{
    if (font)
    {
        return font;
    }

    if (font_key.empty())
    {
        return nullptr;
    }

    return ResourceManager::instance()->find_font(font_key);
}

UiTextTextureResult ui_render_text_texture(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const std::string& text,
    SDL_Color color,
    int wrap_width
)
{
    UiTextTextureResult result;
    if (!renderer || !font || text.empty())
    {
        return result;
    }

    SDL_Surface* text_surface = nullptr;
    if (wrap_width > 0)
    {
        text_surface = TTF_RenderUTF8_Blended_Wrapped(
            font,
            text.c_str(),
            color,
            static_cast<Uint32>(wrap_width)
        );
    }
    else
    {
        text_surface = TTF_RenderUTF8_Blended(
            font,
            text.c_str(),
            color
        );
    }

    if (!text_surface)
    {
        return result;
    }

    result._width = text_surface->w;
    result._height = text_surface->h;
    SDL_Texture* raw_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (!raw_texture)
    {
        result._width = 0;
        result._height = 0;
        return result;
    }

    result._texture.reset(raw_texture);
    SDL_SetTextureBlendMode(result._texture.get(), SDL_BLENDMODE_BLEND);
    return result;
}
