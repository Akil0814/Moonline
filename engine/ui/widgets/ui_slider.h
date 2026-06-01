#pragma once

#include "../bar.h"
#include "../ui_focusable.h"
#include "label.h"

#include <functional>
#include <string>

enum class UiSliderOrientation
{
    Horizontal,
    Vertical
};

using UiSliderValueChangedCallback = std::function<void(float value)>;

class UiSlider : public GameObject, public UiFocusable
{
public:
    explicit UiSlider(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_range(float min_value, float max_value);
    [[nodiscard]] float min_value() const;
    [[nodiscard]] float max_value() const;

    void set_value(float value);
    [[nodiscard]] float value() const;

    void set_step(float step);
    [[nodiscard]] float step() const;

    void set_orientation(UiSliderOrientation orientation);
    [[nodiscard]] UiSliderOrientation orientation() const;

    void set_bar_style(const Bar& style_source);
    Bar& bar();
    [[nodiscard]] const Bar& bar() const;

    void set_thumb_color(SDL_Color color);
    [[nodiscard]] SDL_Color thumb_color() const;

    void set_thumb_size(float thumb_size);
    [[nodiscard]] float thumb_size() const;

    void set_show_value_text(bool show_value_text);
    [[nodiscard]] bool shows_value_text() const;

    void set_value_precision(int precision);
    [[nodiscard]] int value_precision() const;

    void set_value_suffix(const std::string& suffix);
    [[nodiscard]] const std::string& value_suffix() const;

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_text_color(SDL_Color color);
    [[nodiscard]] SDL_Color text_color() const;

    void set_enabled(bool enabled) override;
    [[nodiscard]] bool is_enabled() const override;

    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;

    void set_on_value_changed(UiSliderValueChangedCallback on_value_changed);
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;
    [[nodiscard]] GameObject* game_object() override;
    [[nodiscard]] const GameObject* game_object() const override;

private:
    void sync_value_label();
    void set_value_internal(float value, bool notify);
    [[nodiscard]] float snapped_value(float value) const;
    [[nodiscard]] SDL_Rect track_rect() const;
    [[nodiscard]] SDL_Rect thumb_rect() const;
    [[nodiscard]] bool contains_point(int x, int y) const;
    void update_value_from_point(int x, int y, bool notify);

private:
    Bar _bar;
    Label _value_label;

    UiSliderValueChangedCallback _on_value_changed;

    std::string _font_key = "ui.default";
    std::string _value_suffix;

    SDL_Color _thumb_color{ 232, 240, 250, 255 };
    SDL_Color _text_color{ 244, 244, 248, 255 };

    float _step = 1.0f;
    float _thumb_size = 16.0f;

    int _value_precision = 0;

    bool _enabled = true;
    bool _is_focused = false;
    bool _is_dragging = false;
    bool _was_mouse_down = false;
    bool _show_value_text = false;

    UiSliderOrientation _orientation = UiSliderOrientation::Horizontal;
};
