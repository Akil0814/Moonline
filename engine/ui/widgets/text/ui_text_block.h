#pragma once

#include "../../core/ui_element.h"
#include "../../core/ui_text_align.h"
#include "../../core/ui_text_source.h"
#include "../../style/ui_style.h"
#include "../../style/ui_theme_roles.h"
#include "../../style/ui_visual_styles.h"

#include <string>

struct SDL_Texture;

namespace elysia::ui
{
class UiTextBlock : public UiElement
{
public:
    explicit UiTextBlock(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiTextBlock(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiTextBlock(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiTextBlock() override = default;

    void reset() noexcept override;
    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_text_source(UiTextSource text_source);
    [[nodiscard]] const UiTextSource& text_source() const noexcept;
    void set_text_key(std::string text_key);
    void set_raw_text(std::string raw_text);
    void clear_text();

    void set_style(const UiTextBlockStyle& style) noexcept;
    [[nodiscard]] const UiTextBlockStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept;
    void clear_style_override() noexcept;

    void set_theme_role(UiTextBlockThemeRole role) noexcept;
    [[nodiscard]] UiTextBlockThemeRole theme_role() const noexcept;

    void set_text_point_size(int point_size) noexcept;
    [[nodiscard]] int text_point_size() const noexcept;
    void set_padding(int padding) noexcept;
    [[nodiscard]] int padding() const noexcept;
    void set_horizontal_align(TextHorizontalAlign align) noexcept;
    [[nodiscard]] TextHorizontalAlign horizontal_align() const noexcept;

private:
    [[nodiscard]] bool has_text() const noexcept;
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect text_render_rect(SDL_Texture* text_texture) const noexcept;
    [[nodiscard]] std::string resolved_text() const;
    void apply_theme(const UiTheme& theme) override;

private:
    UiTextSource _text_source;
    UiStyleState<UiTextBlockStyle> _style_state;
    UiTextBlockThemeRole _theme_role = UiTextBlockThemeRole::Default;
    TextHorizontalAlign _horizontal_align = TextHorizontalAlign::Left;
    int _text_point_size = 24;
    int _padding = 0;
};
}
