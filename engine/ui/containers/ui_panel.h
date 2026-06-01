#pragma once

#include "../ui_layout.h"

#include <SDL.h>

#include <string>

class UiPanel : public UILayout
{
public:
    explicit UiPanel(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_render(SDL_Renderer* renderer) override;
    void reset() override;

    void set_draw_background(bool draw_background);
    [[nodiscard]] bool draws_background() const;

    void set_draw_border(bool draw_border);
    [[nodiscard]] bool draws_border() const;

    void set_background_color(SDL_Color color);
    [[nodiscard]] SDL_Color background_color() const;

    void set_border_color(SDL_Color color);
    [[nodiscard]] SDL_Color border_color() const;

    void set_background_texture(SDL_Texture* texture);
    [[nodiscard]] SDL_Texture* background_texture() const;

    void set_background_texture_key(const std::string& texture_key);
    [[nodiscard]] const std::string& background_texture_key() const;

    void set_background_alpha(Uint8 alpha);
    [[nodiscard]] Uint8 background_alpha() const;

    void set_clip_children(bool clip_children);
    [[nodiscard]] bool clips_children() const;

private:
    [[nodiscard]] SDL_Texture* resolve_background_texture() const;

private:
    SDL_Texture* _background_texture = nullptr;
    std::string _background_texture_key;

    SDL_Color _background_color{ 24, 24, 32, 220 };
    SDL_Color _border_color{ 220, 220, 220, 255 };

    Uint8 _background_alpha = 255;

    bool _draw_background = true;
    bool _draw_border = false;
    bool _clip_children = false;
};
