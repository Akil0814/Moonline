#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_element.h"

enum class BarFillDirection
{
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

class UiBar : public UiElement
{
public:
    explicit UiBar(const Rect& rect = Rect::zero(), int order = 0) noexcept;
    UiBar(const Vector2& position, const Vector2& size, int order = 0) noexcept;
    ~UiBar() override = default;

    void reset() noexcept override;

    void set_range(float min_value, float max_value);
    void set_value(float value);
    void set_ratio(float ratio);

    [[nodiscard]] float min_value() const;
    [[nodiscard]] float max_value() const;
    [[nodiscard]] float value() const;
    [[nodiscard]] float ratio() const;

    void set_background_color(Color color);
    [[nodiscard]] Color background_color() const;

    void set_fill_color(Color color);
    [[nodiscard]] Color fill_color() const;

    void set_border_color(Color color);
    [[nodiscard]] Color border_color() const;

    void set_draw_border(bool draw_border);
    [[nodiscard]] bool draws_border() const;

    void set_fill_direction(BarFillDirection direction);
    [[nodiscard]] BarFillDirection fill_direction() const;

    void set_padding(int padding);
    [[nodiscard]] int padding() const;

    void submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const override;

private:
    [[nodiscard]] Rect content_rect(const Rect& rect) const;
    [[nodiscard]] Rect fill_rect(const Rect& rect) const;

private:
    float _min_value = 0.0f;
    float _max_value = 1.0f;
    float _value = 0.0f;

    Color _background_color = colors::loading_blue_bar_background;
    Color _fill_color = colors::loading_blue_bar_fill;
    Color _border_color = colors::black;

    BarFillDirection _fill_direction = BarFillDirection::LeftToRight;
    bool _draw_border = false;
    int _padding = 0;
};
