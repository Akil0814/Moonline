#pragma once

#include "ui_panel.h"

class UiScrollPanel : public UiPanel
{
public:
    explicit UiScrollPanel(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_input_event(const InputEvent& event) override;
    void reset() override;

    void set_scroll_offset(const Vector2& offset);
    [[nodiscard]] const Vector2& scroll_offset() const;

    void scroll_by(const Vector2& delta);
    void set_scroll_step(const Vector2& step);
    [[nodiscard]] const Vector2& scroll_step() const;

    void scroll_left();
    void scroll_right();
    void scroll_up();
    void scroll_down();

    void set_allow_horizontal_scroll(bool allow_horizontal_scroll);
    [[nodiscard]] bool allows_horizontal_scroll() const;

    void set_allow_vertical_scroll(bool allow_vertical_scroll);
    [[nodiscard]] bool allows_vertical_scroll() const;

    void set_clamp_scroll(bool clamp_scroll);
    [[nodiscard]] bool clamps_scroll() const;

    Vector2 max_scroll_offset();
    void ensure_child_visible(const GameObject* child, const Vector2& padding = Vector2::zero());

private:
    void clamp_scroll_offset();
    [[nodiscard]] bool contains_point(int x, int y) const;

private:
    Vector2 _scroll_offset{ 0.0f, 0.0f };
    Vector2 _scroll_step{ 32.0f, 32.0f };

    bool _allow_horizontal_scroll = true;
    bool _allow_vertical_scroll = true;
    bool _clamp_scroll = true;
};
