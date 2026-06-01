#pragma once

#include "../style/ui_theme_roles.h"
#include "../base/ui_element.h"
#include "../../resources/texture/texture_loader.h"

struct UiTheme;

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

enum class TextHorizontalAlign
{
    Left,
    Center,
    Right
};

enum class TextVerticalAlign
{
    Top,
    Center,
    Bottom
};

class UiLabel : public UiElement
{
public:
    explicit UiLabel(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

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

    void set_background_color(SDL_Color color);
    [[nodiscard]] SDL_Color background_color() const;

    void set_draw_background(bool draw_background);
    [[nodiscard]] bool draws_background() const;

    void set_padding(int padding);
    [[nodiscard]] int padding() const;

    void set_auto_size(bool auto_size);
    [[nodiscard]] bool auto_sizes() const;

    void set_horizontal_align(TextHorizontalAlign align);
    [[nodiscard]] TextHorizontalAlign horizontal_align() const;

    void set_vertical_align(TextVerticalAlign align);
    [[nodiscard]] TextVerticalAlign vertical_align() const;

    void set_wrap_width(int wrap_width);
    [[nodiscard]] int wrap_width() const;
    void set_label_theme_role(UiLabelThemeRole label_theme_role);
    [[nodiscard]] UiLabelThemeRole label_theme_role() const;

private:
    void mark_dirty();
    void refresh_texture(SDL_Renderer* renderer);
    [[nodiscard]] TTF_Font* resolve_font() const;
    [[nodiscard]] SDL_Rect text_rect() const;
    void apply_theme(const UiTheme& theme) override;

private:
    std::string _text;
    std::string _font_key;

    TTF_Font* _font = nullptr;
    TexturePtr _texture;

    SDL_Color _text_color{ 255, 255, 255, 255 };
    SDL_Color _background_color{ 0, 0, 0, 0 };

    int _padding = 0;
    int _wrap_width = 0;
    int _texture_width = 0;
    int _texture_height = 0;

    bool _draw_background = false;
    bool _auto_size = true;
    bool _dirty = true;

    TextHorizontalAlign _horizontal_align = TextHorizontalAlign::Left;
    TextVerticalAlign _vertical_align = TextVerticalAlign::Top;
    UiLabelThemeRole _label_theme_role = UiLabelThemeRole::Default;
};
