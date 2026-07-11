#include "ui_label.h"

#include "../../style/ui_style_defaults.h"
#include "../../../core/render/render_command.h"
#include "../../../localization/localization_manager.h"
#include "../../../localization/localized_text_style.h"

#include <SDL.h>

#include <algorithm>
#include <utility>

namespace elysia::ui
{
UiLabel::UiLabel(const elysia::core::Rect& rect,int order,UiTextContent text_content) noexcept
    : UiElement(rect,order)
{
    reset();
    _text_content = std::move(text_content);
}

UiLabel::UiLabel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,
    int order,UiTextContent text_content) noexcept : UiElement(position,size,order)
{
    reset();
    _text_content = std::move(text_content);
}

UiLabel::UiLabel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,
    UiFromCenterTag,int order,UiTextContent text_content) noexcept : UiElement(center,size,from_center,order)
{
    reset();
    _text_content = std::move(text_content);
}

void UiLabel::reset() noexcept
{
    UiElement::reset();
    _text_content = UiTextContent{};
    _style_state.reset(UiStyleDefaults::label());
    _visual_role = UiLabelVisualRole::Default;
    _typography_role = UiTypographyRole::Label;
    _target_height.reset();
    _horizontal_align = resolve_ui_typography(_typography_role).horizontal_align_default;
    _vertical_align = TextVerticalAlign::Top;
    _padding = 0;
}

void UiLabel::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect& label_rect = screen_rect();
    if (label_rect.is_empty())
        return;

    const UiLabelStyle& style = _style_state.effective_style();
    if (style.draw_background)
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(label_rect,apply_opacity(style.background),style.corner_radius));

    if (_text_content.empty())
        return;

    elysia::localization::LocalizationManager* localization_manager = elysia::localization::LocalizationManager::instance();
    if (!localization_manager)
        return;

    const UiResolvedTextStyle typography = resolve_ui_typography(_typography_role);
    elysia::localization::LocalizedTextStyle text_style;
    text_style.point_size = typography.point_size;
    text_style.color = style.text;
    text_style.wrap_width = typography.wrap_allowed ? std::max(0,static_cast<int>(content_rect().width())) : 0;

    SDL_Texture* text_texture = nullptr;
    if (_text_content.kind == UiTextContentKind::TextKey)
        text_texture = localization_manager->get_text_texture(_text_content.value,text_style);
    else if (_text_content.kind == UiTextContentKind::RawText)
        text_texture = localization_manager->get_raw_text_texture(_text_content.value,text_style);
    if (!text_texture)
        return;

    const elysia::core::Rect text_rect = text_render_rect(text_texture);
    if (text_rect.is_empty())
        return;

    elysia::core::UiRenderCommand command = elysia::core::make_ui_texture_command(text_texture,text_rect);
    apply_opacity(command);
    out_commands.push_back(command);
}

void UiLabel::set_text_content(UiTextContent text_content)
{
    _text_content = std::move(text_content);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

const UiTextContent& UiLabel::text_content() const noexcept
{
    return _text_content;
}

void UiLabel::set_typography_role(UiTypographyRole role) noexcept
{
    _typography_role = role;
    _horizontal_align = resolve_ui_typography(_typography_role).horizontal_align_default;
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

UiTypographyRole UiLabel::typography_role() const noexcept
{
    return _typography_role;
}

void UiLabel::set_target_height(float height)
{
    _target_height = std::max(0.0f,height);
}

std::optional<float> UiLabel::target_height() const noexcept
{
    return _target_height;
}

void UiLabel::clear_target_height()
{
    _target_height.reset();
}

void UiLabel::set_base_style(const UiLabelStyle& style) noexcept
{
    _style_state.set_base_style(style);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

void UiLabel::set_style(const UiLabelStyle& style) noexcept
{
    _style_state.set_style_override(style);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

const UiLabelStyle& UiLabel::style() const noexcept
{
    return _style_state.effective_style();
}

bool UiLabel::has_style_override() const noexcept
{
    return _style_state.has_style_override();
}

void UiLabel::clear_style_override() noexcept
{
    _style_state.clear_style_override();
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

void UiLabel::set_visual_role(UiLabelVisualRole role) noexcept
{
    _visual_role = role;
    notify_base_style_invalidated();
}

UiLabelVisualRole UiLabel::visual_role() const noexcept
{
    return _visual_role;
}

void UiLabel::set_horizontal_align(TextHorizontalAlign align)
{
    _horizontal_align = align;
}

TextHorizontalAlign UiLabel::horizontal_align() const noexcept
{
    return _horizontal_align;
}

void UiLabel::set_vertical_align(TextVerticalAlign align)
{
    _vertical_align = align;
}

TextVerticalAlign UiLabel::vertical_align() const noexcept
{
    return _vertical_align;
}

void UiLabel::set_padding(int padding)
{
    _padding = std::max(0,padding);
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

int UiLabel::padding() const noexcept
{
    return _padding;
}

elysia::core::Rect UiLabel::content_rect() const noexcept
{
    const elysia::core::Rect& label_rect = screen_rect();
    const float width = std::max(0.0f,label_rect.width());
    const float height = std::max(0.0f,label_rect.height());
    const float padding = static_cast<float>(_padding);
    const float pad_x = std::min(padding,width * 0.5f);
    const float pad_y = std::min(padding,height * 0.5f);

    elysia::core::Rect content = label_rect;
    content.set_x(label_rect.x() + pad_x);
    content.set_y(label_rect.y() + pad_y);
    content.set_width(width - pad_x * 2.0f);
    content.set_height(height - pad_y * 2.0f);
    return content;
}

elysia::core::Rect UiLabel::text_render_rect(SDL_Texture* text_texture) const noexcept
{
    if (!text_texture)
        return elysia::core::Rect::zero();

    int texture_width = 0;
    int texture_height = 0;
    if (SDL_QueryTexture(text_texture,nullptr,nullptr,&texture_width,&texture_height) != 0)
        return elysia::core::Rect::zero();
    if (texture_width <= 0 || texture_height <= 0)
        return elysia::core::Rect::zero();

    const elysia::core::Rect available_rect = content_rect();
    if (available_rect.is_empty())
        return elysia::core::Rect::zero();

    const float available_width = available_rect.width();
    const float available_height = available_rect.height();
    const float width_scale = available_width / static_cast<float>(texture_width);
    const float height_scale = available_height / static_cast<float>(texture_height);
    const float target_scale = _target_height.has_value() && *_target_height > 0.0f
        ? *_target_height / static_cast<float>(texture_height)
        : 1.0f;
    const float scale = std::min(target_scale,std::min(width_scale,height_scale));
    const elysia::core::Vector2 render_size(
        static_cast<float>(texture_width) * scale,
        static_cast<float>(texture_height) * scale
    );

    float x = available_rect.x();
    switch (_horizontal_align)
    {
    case TextHorizontalAlign::Center:
        x = available_rect.center().x - render_size.x * 0.5f;
        break;
    case TextHorizontalAlign::Right:
        x = available_rect.right() - render_size.x;
        break;
    case TextHorizontalAlign::Left:
    default:
        x = available_rect.x();
        break;
    }

    float y = available_rect.y();
    switch (_vertical_align)
    {
    case TextVerticalAlign::Center:
        y = available_rect.center().y - render_size.y * 0.5f;
        break;
    case TextVerticalAlign::Bottom:
        y = available_rect.bottom() - render_size.y;
        break;
    case TextVerticalAlign::Top:
    default:
        y = available_rect.y();
        break;
    }

    return elysia::core::Rect(x,y,render_size.x,render_size.y);
}

}

