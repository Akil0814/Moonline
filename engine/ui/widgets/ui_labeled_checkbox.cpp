#include "ui_labeled_checkbox.h"

#include "../style/ui_style_defaults.h"
#include "../style/ui_theme.h"
#include "../../core/render/render_command.h"

#include <algorithm>
#include <utility>

namespace elysia::ui
{
namespace
{
[[nodiscard]] float clamp_non_negative(float value) noexcept
{
    return std::max(0.0f,value);
}
}

UiLabeledCheckbox::UiLabeledCheckbox(const elysia::core::Rect& rect,int order) noexcept
    : UiCheckbox(rect,order)
{
    reset();
}

UiLabeledCheckbox::UiLabeledCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiLabeledCheckbox(elysia::core::Rect(position.x,position.y,size.x,size.y),order) {}

UiLabeledCheckbox::UiLabeledCheckbox(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiLabeledCheckbox(elysia::core::Rect::from_center(center,size),order) {}

UiLabeledCheckbox::UiLabeledCheckbox(const elysia::core::Rect& rect,const UiLabeledCheckboxConfig& config,int order) noexcept
    : UiLabeledCheckbox(rect,order)
{
    set_labeled_checkbox_config(config);
}

UiLabeledCheckbox::UiLabeledCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiLabeledCheckboxConfig& config,int order) noexcept
    : UiLabeledCheckbox(elysia::core::Rect(position.x,position.y,size.x,size.y),config,order) {}

UiLabeledCheckbox::UiLabeledCheckbox(
    const elysia::core::Vector2& center,const elysia::core::Vector2& size,
    UiFromCenterTag,const UiLabeledCheckboxConfig& config,int order
) noexcept : UiLabeledCheckbox(elysia::core::Rect::from_center(center,size),config,order) {}

void UiLabeledCheckbox::reset() noexcept
{
    UiCheckbox::reset();
    _label.reset();
    _label.set_use_theme(false);
    _label.set_style(UiLabelStyle{
        .text = UiStyleDefaults::label().text,
        .background = elysia::core::colors::transparent,
        .draw_background = false
    });
    _label.set_typography_role(UiTypographyRole::CheckboxLabel);
    _label.set_horizontal_align(TextHorizontalAlign::Left);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_padding(0);

    _text_content = UiTextContent{};
    _typography_role = UiTypographyRole::CheckboxLabel;
    _label_placement = UiLabeledCheckboxLabelPlacement::Right;
    _label_spacing = 8.0f;
    _text_placement = UiLabeledCheckboxTextPlacement::NearBox;
    const UiEnabledDisabledColors text_colors = UiStyleDefaults::labeled_checkbox_text();
    _text_color = text_colors.enabled;
    _disabled_text_color = text_colors.disabled;
    _draw_background = false;
    _draw_border = false;
}

void UiLabeledCheckbox::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect& rect = screen_rect();
    if (!rect.is_empty())
    {
        if (_draw_background)
            out_commands.push_back(elysia::core::make_ui_fill_rect_command(rect,apply_opacity(current_background_color())));
        if (_draw_border)
            out_commands.push_back(elysia::core::make_ui_draw_rect_command(rect,apply_opacity(current_border_color())));
    }

    UiCheckbox::submit_ui_render_commands(out_commands);
    sync_label_visuals();
    _label.submit_ui_render_commands(out_commands);
}

void UiLabeledCheckbox::set_labeled_checkbox_config(const UiLabeledCheckboxConfig& config)
{
    apply_labeled_checkbox_config(config);
}

void UiLabeledCheckbox::set_text_content(UiTextContent text_content)
{
    _text_content = std::move(text_content);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

const UiTextContent& UiLabeledCheckbox::text_content() const noexcept
{
    return _text_content;
}

void UiLabeledCheckbox::set_label_placement(UiLabeledCheckboxLabelPlacement placement) noexcept
{
    _label_placement = placement;
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

UiLabeledCheckboxLabelPlacement UiLabeledCheckbox::label_placement() const noexcept
{
    return _label_placement;
}

void UiLabeledCheckbox::set_label_spacing(float spacing) noexcept
{
    _label_spacing = clamp_non_negative(spacing);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

float UiLabeledCheckbox::label_spacing() const noexcept
{
    return _label_spacing;
}

void UiLabeledCheckbox::set_text_placement(UiLabeledCheckboxTextPlacement placement) noexcept
{
    _text_placement = placement;
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

UiLabeledCheckboxTextPlacement UiLabeledCheckbox::text_placement() const noexcept
{
    return _text_placement;
}

void UiLabeledCheckbox::set_typography_role(UiTypographyRole role) noexcept
{
    _typography_role = role;
    _label.set_typography_role(role);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

UiTypographyRole UiLabeledCheckbox::typography_role() const noexcept
{
    return _typography_role;
}

void UiLabeledCheckbox::set_label_padding(int padding) noexcept
{
    _label.set_padding(padding);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

int UiLabeledCheckbox::label_padding() const noexcept
{
    return _label.padding();
}

elysia::core::Rect UiLabeledCheckbox::checkbox_rect() const noexcept
{
    const elysia::core::Rect content = content_rect();
    if (content.is_empty())
        return elysia::core::Rect::zero();

    const float side = std::min(content.width(),content.height());
    if (side <= 0.0f)
        return elysia::core::Rect::zero();

    const float y = content.center().y - side * 0.5f;
    if (_label_placement == UiLabeledCheckboxLabelPlacement::Left)
        return elysia::core::Rect(content.right() - side,y,side,side);
    return elysia::core::Rect(content.x(),y,side,side);
}

void UiLabeledCheckbox::apply_labeled_checkbox_config(const UiLabeledCheckboxConfig& config)
{
    set_checkbox_config(config.checkbox);
    set_text_content(config.text_content);
    set_label_placement(config.label_placement);
    set_label_spacing(config.label_spacing);
    set_text_placement(config.text_placement);
    if (config.text_colors)
    {
        _text_color = config.text_colors->enabled;
        _disabled_text_color = config.text_colors->disabled;
    }
    _draw_background = config.draw_background;
    _draw_border = config.draw_border;
}

void UiLabeledCheckbox::sync_label_visuals() const
{
    UiLabelStyle label_style = UiStyleDefaults::label();
    label_style.text = is_enabled() ? _text_color : _disabled_text_color;
    label_style.draw_background = false;

    _label.set_screen_rect(label_rect());
    _label.set_visible(!_text_content.empty() && !_label.screen_rect().is_empty() && is_visible());
    _label.set_opacity(opacity());
    _label.set_text_content(_text_content);
    _label.set_typography_role(_typography_role);
    _label.set_style(label_style);
    _label.set_vertical_align(TextVerticalAlign::Center);
    if (_label_placement == UiLabeledCheckboxLabelPlacement::Left)
    {
        _label.set_horizontal_align(
            _text_placement == UiLabeledCheckboxTextPlacement::NearBox
                ? TextHorizontalAlign::Right
                : TextHorizontalAlign::Left);
    }
    else
    {
        _label.set_horizontal_align(
            _text_placement == UiLabeledCheckboxTextPlacement::NearBox
                ? TextHorizontalAlign::Left
                : TextHorizontalAlign::Right);
    }
}

elysia::core::Rect UiLabeledCheckbox::label_rect() const noexcept
{
    const elysia::core::Rect content = content_rect();
    const elysia::core::Rect box = checkbox_rect();
    if (content.is_empty() || box.is_empty())
        return elysia::core::Rect::zero();

    const float spacing = _text_content.empty() ? 0.0f : _label_spacing;
    if (_label_placement == UiLabeledCheckboxLabelPlacement::Left)
    {
        const float width = std::max(0.0f,box.x() - content.x() - spacing);
        return elysia::core::Rect(content.x(),content.y(),width,content.height());
    }

    const float x = box.right() + spacing;
    const float width = std::max(0.0f,content.right() - x);
    return elysia::core::Rect(x,content.y(),width,content.height());
}

void UiLabeledCheckbox::apply_theme(const UiTheme& theme)
{
    // The composite owns one registration and forwards the resolved checkbox + label visuals to
    // its private subcontrols instead of exposing them to the theme manager individually.
    UiCheckbox::apply_theme(theme);
    _text_color = theme.label(UiLabelThemeRole::Default).text;
    _disabled_text_color = theme.button(UiButtonThemeRole::Default).text.disabled;
}
}
