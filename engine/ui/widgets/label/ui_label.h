#pragma once

#include "../../style/ui_style.h"
#include "../../style/ui_visual_roles.h"
#include "../../style/ui_visual_styles.h"
#include "../../core/ui_element.h"
#include "../../core/ui_text_align.h"
#include "../../text/ui_text_content.h"
#include "../../text/ui_typography.h"

#include <optional>
#include <string>

struct SDL_Texture;

namespace elysia::ui
{
// Single-line text widget. Typography chooses the source font while target height only scales output.
class UiLabel : public UiElement
{
public:
    explicit UiLabel(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0,UiTextContent text_content = {}) noexcept;

    UiLabel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,
        int order = 0,UiTextContent text_content = {}) noexcept;

    UiLabel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,
        UiFromCenterTag,int order = 0,UiTextContent text_content = {}) noexcept;

    ~UiLabel() override = default;

    void reset() noexcept override;
    // Emits background and text commands for the current label content and alignment.
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_text_content(UiTextContent text_content);
    [[nodiscard]] const UiTextContent& text_content() const noexcept;

    void set_base_style(const UiLabelStyle& style) noexcept;
    void set_style_overrides(const UiLabelStyleOverrides& overrides) noexcept;
    [[nodiscard]] const UiLabelStyle& style() const noexcept;
    [[nodiscard]] const UiLabelStyleOverrides& style_overrides() const noexcept;
    [[nodiscard]] bool has_style_overrides() const noexcept;
    void clear_style_overrides() noexcept;

    void set_visual_role(UiLabelVisualRole role) noexcept;
    [[nodiscard]] UiLabelVisualRole visual_role() const noexcept;

    void set_horizontal_align(TextHorizontalAlign align);
    [[nodiscard]] TextHorizontalAlign horizontal_align() const noexcept;

    void set_vertical_align(TextVerticalAlign align);
    [[nodiscard]] TextVerticalAlign vertical_align() const noexcept;

    void set_typography_role(UiTypographyRole role) noexcept;
    [[nodiscard]] UiTypographyRole typography_role() const noexcept;

    // Optional display height preserves aspect ratio and remains bounded by the label's content rect.
    void set_target_height(float height);
    [[nodiscard]] std::optional<float> target_height() const noexcept;
    void clear_target_height();

    void set_padding(int padding);
    [[nodiscard]] int padding() const noexcept;

private:
    // Returns the padded interior used to position rendered text.
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    // Fits rendered text into the content rect using the active alignment and padding.
    [[nodiscard]] elysia::core::Rect text_render_rect(SDL_Texture* text_texture) const noexcept;

private:
    UiTextContent _text_content;
    UiStyleState<UiLabelStyle> _style_state;
    UiLabelVisualRole _visual_role = UiLabelVisualRole::Default;
    UiTypographyRole _typography_role = UiTypographyRole::Label;
    std::optional<float> _target_height;
    TextHorizontalAlign _horizontal_align = TextHorizontalAlign::Left;
    TextVerticalAlign _vertical_align = TextVerticalAlign::Top;
    int _padding = 0;
};
}
