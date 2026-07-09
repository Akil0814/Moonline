#pragma once

#include "../../style/ui_style.h"
#include "../../style/ui_theme_roles.h"
#include "../../style/ui_visual_styles.h"
#include "../../core/ui_element.h"
#include "../../core/ui_text_align.h"

#include <string>

struct SDL_Texture;

namespace elysia::ui
{
class UiLabel : public UiElement
{
public:
    explicit UiLabel(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0,std::string text_key = {}) noexcept;

    UiLabel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,
        int order = 0,std::string text_key = {}) noexcept;

    UiLabel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,
        UiFromCenterTag,int order = 0,std::string text_key = {}) noexcept;

    ~UiLabel() override = default;

    void reset() noexcept override;
    // Emits background and text commands for the current label content and alignment.
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_style(const UiLabelStyle& style) noexcept;
    [[nodiscard]] const UiLabelStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept;
    void clear_style_override() noexcept;

    void set_theme_role(UiLabelThemeRole role) noexcept;
    [[nodiscard]] UiLabelThemeRole theme_role() const noexcept;

    void set_text_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color text_color() const noexcept;

    void set_background_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color background_color() const noexcept;

    void set_draw_background(bool draw_background);
    [[nodiscard]] bool draws_background() const noexcept;

    void set_horizontal_align(TextHorizontalAlign align);
    [[nodiscard]] TextHorizontalAlign horizontal_align() const noexcept;

    void set_vertical_align(TextVerticalAlign align);
    [[nodiscard]] TextVerticalAlign vertical_align() const noexcept;

    void set_text_point_size(int point_size);
    [[nodiscard]] int text_point_size() const noexcept;

    void set_padding(int padding);
    [[nodiscard]] int padding() const noexcept;

private:
    // Returns the padded interior used to position rendered text.
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    // Fits rendered text into the content rect using the active alignment and padding.
    [[nodiscard]] elysia::core::Rect text_render_rect(SDL_Texture* text_texture) const noexcept;
    void apply_theme(const UiTheme& theme) override;

private:
    std::string _text_key;
    UiStyleState<UiLabelStyle> _style_state;
    UiLabelThemeRole _theme_role = UiLabelThemeRole::Default;
    TextHorizontalAlign _horizontal_align = TextHorizontalAlign::Left;
    TextVerticalAlign _vertical_align = TextVerticalAlign::Top;
    int _text_point_size = 24;
    int _padding = 0;
};
}
