#include "ui_labeled_checkbox.h"

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
    : UiCheckbox(rect,order) {}

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
    _label.set_draw_background(false);
    _label.set_horizontal_align(TextHorizontalAlign::Left);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_padding(0);

    _text_key.clear();
    _label_placement = UiLabeledCheckboxLabelPlacement::Right;
    _label_spacing = 8.0f;
    _text_color = elysia::core::colors::white;
    _disabled_text_color = elysia::core::colors::gray_300;
}

void UiLabeledCheckbox::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    UiCheckbox::submit_ui_render_commands(out_commands);
    sync_label_visuals();
    _label.submit_ui_render_commands(out_commands);
}

void UiLabeledCheckbox::set_labeled_checkbox_config(const UiLabeledCheckboxConfig& config)
{
    apply_labeled_checkbox_config(config);
}

void UiLabeledCheckbox::set_text_key(std::string text_key)
{
    _text_key = std::move(text_key);
}

const std::string& UiLabeledCheckbox::text_key() const noexcept
{
    return _text_key;
}

void UiLabeledCheckbox::set_label_placement(UiLabeledCheckboxLabelPlacement placement) noexcept
{
    _label_placement = placement;
}

UiLabeledCheckboxLabelPlacement UiLabeledCheckbox::label_placement() const noexcept
{
    return _label_placement;
}

void UiLabeledCheckbox::set_label_spacing(float spacing) noexcept
{
    _label_spacing = clamp_non_negative(spacing);
}

float UiLabeledCheckbox::label_spacing() const noexcept
{
    return _label_spacing;
}

void UiLabeledCheckbox::set_text_color(elysia::core::Color color) noexcept
{
    _text_color = color;
}

elysia::core::Color UiLabeledCheckbox::text_color() const noexcept
{
    return _text_color;
}

void UiLabeledCheckbox::set_disabled_text_color(elysia::core::Color color) noexcept
{
    _disabled_text_color = color;
}

elysia::core::Color UiLabeledCheckbox::disabled_text_color() const noexcept
{
    return _disabled_text_color;
}

void UiLabeledCheckbox::set_text_point_size(int point_size) noexcept
{
    _label.set_text_point_size(point_size);
}

int UiLabeledCheckbox::text_point_size() const noexcept
{
    return _label.text_point_size();
}

void UiLabeledCheckbox::set_label_padding(int padding) noexcept
{
    _label.set_padding(padding);
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
    set_text_key(config.text_key);
    set_label_placement(config.label_placement);
    set_label_spacing(config.label_spacing);
}

void UiLabeledCheckbox::sync_label_visuals() const
{
    _label.set_screen_rect(label_rect());
    _label.set_visible(!_text_key.empty() && !_label.screen_rect().is_empty() && is_visible());
    _label.set_opacity(opacity());
    _label.set_text_key(_text_key);
    _label.set_text_color(is_enabled() ? _text_color : _disabled_text_color);
    _label.set_draw_background(false);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_horizontal_align(_label_placement == UiLabeledCheckboxLabelPlacement::Left ? TextHorizontalAlign::Right : TextHorizontalAlign::Left);
}

elysia::core::Rect UiLabeledCheckbox::label_rect() const noexcept
{
    const elysia::core::Rect content = content_rect();
    const elysia::core::Rect box = checkbox_rect();
    if (content.is_empty() || box.is_empty())
        return elysia::core::Rect::zero();

    const float spacing = _text_key.empty() ? 0.0f : _label_spacing;
    if (_label_placement == UiLabeledCheckboxLabelPlacement::Left)
    {
        const float width = std::max(0.0f,box.x() - content.x() - spacing);
        return elysia::core::Rect(content.x(),content.y(),width,content.height());
    }

    const float x = box.right() + spacing;
    const float width = std::max(0.0f,content.right() - x);
    return elysia::core::Rect(x,content.y(),width,content.height());
}
}
