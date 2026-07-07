#pragma once

#include "../style/ui_visual_styles.h"
#include "../core/ui_element.h"

namespace elysia::ui
{
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
    explicit UiBar(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiBar(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiBar(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiBar() override = default;

    void reset() noexcept override;

    void set_range(float min_value,float max_value);
    void set_value(float value);
    void set_ratio(float ratio);

    [[nodiscard]] float min_value() const;
    [[nodiscard]] float max_value() const;
    [[nodiscard]] float value() const;
    [[nodiscard]] float ratio() const;

    void set_style(const UiBarStyle& style) noexcept;
    [[nodiscard]] const UiBarStyle& style() const noexcept;

    void set_background_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color background_color() const;

    void set_fill_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color fill_color() const;

    void set_border_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color border_color() const;

    void set_draw_border(bool draw_border);
    [[nodiscard]] bool draws_border() const;

    void set_fill_direction(BarFillDirection direction);
    [[nodiscard]] BarFillDirection fill_direction() const;

    void set_padding(int padding);
    [[nodiscard]] int padding() const;

    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

private:
    [[nodiscard]] elysia::core::Rect content_rect(const elysia::core::Rect& rect) const;
    [[nodiscard]] elysia::core::Rect fill_rect(const elysia::core::Rect& rect) const;

private:
    float _min_value = 0.0f;
    float _max_value = 1.0f;
    float _value = 0.0f;
    UiBarStyle _style{};
    BarFillDirection _fill_direction = BarFillDirection::LeftToRight;
    int _padding = 0;
};

}
