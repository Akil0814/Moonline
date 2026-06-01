#pragma once

#include "../ui_control.h"
#include "ui_label.h"

#include <functional>
#include <string>

using UiToggleChangedCallback = std::function<void(bool value)>;

class UiToggle : public UiControl
{
public:
    explicit UiToggle(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_value(bool value);
    [[nodiscard]] bool value() const;

    void set_state_texts(const std::string& off_text, const std::string& on_text);
    [[nodiscard]] const std::string& off_text() const;
    [[nodiscard]] const std::string& on_text() const;
    [[nodiscard]] const std::string& display_text() const;

    void set_label_text(const std::string& label_text);
    [[nodiscard]] const std::string& label_text() const;

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_text_color(SDL_Color color);
    [[nodiscard]] SDL_Color text_color() const;

    void set_background_color(SDL_Color color);
    [[nodiscard]] SDL_Color background_color() const;

    void set_focused_background_color(SDL_Color color);
    [[nodiscard]] SDL_Color focused_background_color() const;

    void set_border_color(SDL_Color color);
    [[nodiscard]] SDL_Color border_color() const;

    void set_on_changed(UiToggleChangedCallback on_changed);
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;

private:
    void toggle();
    void set_value_internal(bool value, bool notify);
    void sync_labels();
    [[nodiscard]] bool contains_point(int x, int y) const;
    void apply_theme(const UiTheme& theme) override;

private:
    UiLabel _label;
    UiLabel _value_label;

    UiToggleChangedCallback _on_changed;

    std::string _label_text;
    std::string _off_text = "Off";
    std::string _on_text = "On";
    std::string _font_key = "ui.default";

    SDL_Color _text_color{ 244, 244, 248, 255 };
    SDL_Color _background_color{ 32, 42, 58, 220 };
    SDL_Color _focused_background_color{ 52, 78, 116, 235 };
    SDL_Color _border_color{ 112, 140, 180, 255 };

    bool _value = false;
    bool _was_mouse_down = false;
};
