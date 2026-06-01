#pragma once

#include "../ui_control.h"
#include "../../resources/texture/texture_loader.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <functional>
#include <string>

using UiTextInputChangedCallback = std::function<void(const std::string& text)>;

class UiTextInput : public UiControl
{
public:
    explicit UiTextInput(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_text(const std::string& text);
    [[nodiscard]] const std::string& text() const;

    void set_placeholder_text(const std::string& placeholder_text);
    [[nodiscard]] const std::string& placeholder_text() const;

    void set_font(TTF_Font* font);
    [[nodiscard]] TTF_Font* font() const;

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_max_length(size_t max_length);
    [[nodiscard]] size_t max_length() const;

    void set_read_only(bool read_only);
    [[nodiscard]] bool is_read_only() const;

    void set_password_mode(bool password_mode);
    [[nodiscard]] bool is_password_mode() const;
    void set_enabled(bool enabled) override;
    void set_focused(bool focused) override;

    void set_padding(int padding);
    [[nodiscard]] int padding() const;

    void set_text_color(SDL_Color color);
    [[nodiscard]] SDL_Color text_color() const;

    void set_placeholder_color(SDL_Color color);
    [[nodiscard]] SDL_Color placeholder_color() const;

    void set_background_color(SDL_Color color);
    [[nodiscard]] SDL_Color background_color() const;

    void set_border_color(SDL_Color color);
    [[nodiscard]] SDL_Color border_color() const;

    void set_focused_border_color(SDL_Color color);
    [[nodiscard]] SDL_Color focused_border_color() const;

    void set_caret_color(SDL_Color color);
    [[nodiscard]] SDL_Color caret_color() const;

    void set_on_text_changed(UiTextInputChangedCallback on_text_changed);
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;

private:
    void mark_dirty();
    void refresh_texture(SDL_Renderer* renderer);
    [[nodiscard]] TTF_Font* resolve_font() const;
    [[nodiscard]] std::string display_text() const;
    void set_text_internal(const std::string& text, bool notify);
    void move_caret_left();
    void move_caret_right();
    void insert_text(const std::string& text);
    void erase_left();
    void erase_right();
    void update_caret_from_mouse(int mouse_x);
    [[nodiscard]] int caret_pixel_x() const;
    [[nodiscard]] bool contains_point(int x, int y) const;
    void apply_theme(const UiTheme& theme) override;

private:
    std::string _text;
    std::string _placeholder_text;
    std::string _font_key = "ui.default";

    TTF_Font* _font = nullptr;
    TexturePtr _texture;

    UiTextInputChangedCallback _on_text_changed;

    SDL_Color _text_color{ 244, 244, 248, 255 };
    SDL_Color _placeholder_color{ 142, 154, 170, 255 };
    SDL_Color _background_color{ 28, 36, 52, 220 };
    SDL_Color _border_color{ 110, 140, 182, 255 };
    SDL_Color _focused_border_color{ 166, 208, 255, 255 };
    SDL_Color _caret_color{ 248, 248, 252, 255 };

    size_t _max_length = 64;
    size_t _caret_index = 0;

    int _padding = 10;
    int _texture_width = 0;
    int _texture_height = 0;

    double _caret_timer = 0.0;

    bool _read_only = false;
    bool _password_mode = false;
    bool _show_caret = true;
    bool _dirty = true;
    bool _was_mouse_down = false;
};
