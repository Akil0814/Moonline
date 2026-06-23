#include "ui_label.h"

#include "../../core/render/render_command.h"
#include "../../localization/localization_manager.h"
#include "../../localization/localized_text_style.h"

#include <SDL.h>

#include <algorithm>
#include <utility>

namespace elysia::ui
{
UiLabel::UiLabel(const elysia::core::Rect& rect, int order, std::string text_key) noexcept
    : UiElement(rect, order),
      _text_key(std::move(text_key))
{
    set_use_theme(false);
}

UiLabel::UiLabel(
    const elysia::core::Vector2& position,
    const elysia::core::Vector2& size,
    int order,
    std::string text_key
) noexcept
    : UiElement(position, size, order),
      _text_key(std::move(text_key))
{
    set_use_theme(false);
}

void UiLabel::reset() noexcept
{
    UiElement::reset();
    set_use_theme(false);

    _text_key.clear();
    _text_color = elysia::core::colors::white;
    _background_color = elysia::core::colors::transparent;
    _horizontal_align = TextHorizontalAlign::Left;
    _vertical_align = TextVerticalAlign::Top;
    _text_point_size = 24;
    _padding = 0;
    _draw_background = false;
}

void UiLabel::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
    {
        return;
    }

    const elysia::core::Rect& label_rect = screen_rect();
    if (label_rect.is_empty())
    {
        return;
    }

    if (_draw_background)
    {
        out_commands.push_back(
            elysia::core::make_ui_fill_rect_command(label_rect, apply_opacity(_background_color))
        );
    }

    if (_text_key.empty())
    {
        return;
    }

    elysia::localization::LocalizationManager* localization_manager = elysia::localization::LocalizationManager::instance();
    if (!localization_manager)
    {
        return;
    }

    elysia::localization::LocalizedTextStyle text_style;
    text_style.point_size = _text_point_size;
    text_style.color = _text_color;
    text_style.wrap_width = std::max(0, static_cast<int>(content_rect().width()));

    SDL_Texture* text_texture = localization_manager->get_text_texture(_text_key, text_style);
    if (!text_texture)
    {
        return;
    }

    const elysia::core::Rect text_rect = text_render_rect(text_texture);
    if (text_rect.is_empty())
    {
        return;
    }

    elysia::core::UiRenderCommand command = elysia::core::make_ui_texture_command(text_texture, text_rect);
    apply_opacity(command);
    out_commands.push_back(command);
}

void UiLabel::set_text_key(std::string text_key)
{
    _text_key = std::move(text_key);
}

const std::string& UiLabel::text_key() const noexcept
{
    return _text_key;
}

void UiLabel::set_text_color(elysia::core::Color color)
{
    _text_color = color;
}

elysia::core::Color UiLabel::text_color() const noexcept
{
    return _text_color;
}

void UiLabel::set_background_color(elysia::core::Color color)
{
    _background_color = color;
}

elysia::core::Color UiLabel::background_color() const noexcept
{
    return _background_color;
}

void UiLabel::set_draw_background(bool draw_background)
{
    _draw_background = draw_background;
}

bool UiLabel::draws_background() const noexcept
{
    return _draw_background;
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

void UiLabel::set_text_point_size(int point_size)
{
    _text_point_size = std::max(0, point_size);
}

int UiLabel::text_point_size() const noexcept
{
    return _text_point_size;
}

void UiLabel::set_padding(int padding)
{
    _padding = std::max(0, padding);
}

int UiLabel::padding() const noexcept
{
    return _padding;
}

elysia::core::Rect UiLabel::content_rect() const noexcept
{
    const elysia::core::Rect& label_rect = screen_rect();
    const float width = std::max(0.0f, label_rect.width());
    const float height = std::max(0.0f, label_rect.height());

    const float padding = static_cast<float>(_padding);
    const float pad_x = std::min(padding, width * 0.5f);
    const float pad_y = std::min(padding, height * 0.5f);

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
    {
        return elysia::core::Rect::zero();
    }

    int texture_width = 0;
    int texture_height = 0;
    if (SDL_QueryTexture(text_texture, nullptr, nullptr, &texture_width, &texture_height) != 0)
    {
        return elysia::core::Rect::zero();
    }

    if (texture_width <= 0 || texture_height <= 0)
    {
        return elysia::core::Rect::zero();
    }

    const elysia::core::Rect available_rect = content_rect();
    if (available_rect.is_empty())
    {
        return elysia::core::Rect::zero();
    }

    const float available_width = available_rect.width();
    const float available_height = available_rect.height();
    const float width_scale = available_width / static_cast<float>(texture_width);
    const float height_scale = available_height / static_cast<float>(texture_height);
    const float scale = std::min(1.0f, std::min(width_scale, height_scale));

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

    return elysia::core::Rect(x, y, render_size.x, render_size.y);
}

}