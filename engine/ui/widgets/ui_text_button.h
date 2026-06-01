#pragma once

#include "../ui_button.h"
#include "../../resources/texture/texture_loader.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

class UiTextButton : public UiButton
{
public:
    explicit UiTextButton(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_render(SDL_Renderer* renderer) override;
    void reset() override;

    void set_text(const std::string& text);
    [[nodiscard]] const std::string& text() const;

    void set_font(TTF_Font* font);
    [[nodiscard]] TTF_Font* font() const;

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_text_color(SDL_Color color);
    [[nodiscard]] SDL_Color text_color() const;

    void set_wrap_width(int wrap_width);
    [[nodiscard]] int wrap_width() const;

private:
    void mark_dirty();
    void refresh_text_texture(SDL_Renderer* renderer);
    [[nodiscard]] TTF_Font* resolve_font() const;

private:
    std::string _text;
    std::string _font_key;

    TTF_Font* _font = nullptr;
    TexturePtr _message_texture;

    SDL_Color _text_color{ 255, 255, 255, 255 };

    int _wrap_width = 0;
    bool _dirty = true;
};
