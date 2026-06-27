#include "ui_panel.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
[[nodiscard]] elysia::core::Vector2 clamp_size(const elysia::core::Vector2& size) noexcept
{
    return elysia::core::Vector2(std::max(0.0f,size.x),std::max(0.0f,size.y));
}

[[nodiscard]] float horizontal_margin_offset(UiLayoutAnchor anchor,const UiLayoutMargin& margin) noexcept
{
    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
    case UiLayoutAnchor::CenterLeft:
    case UiLayoutAnchor::BottomLeft:
        return margin.left;
    case UiLayoutAnchor::TopCenter:
    case UiLayoutAnchor::Center:
    case UiLayoutAnchor::BottomCenter:
        return margin.left - margin.right;
    case UiLayoutAnchor::TopRight:
    case UiLayoutAnchor::CenterRight:
    case UiLayoutAnchor::BottomRight:
        return -margin.right;
    default:
        return 0.0f;
    }
}

[[nodiscard]] float vertical_margin_offset(UiLayoutAnchor anchor,const UiLayoutMargin& margin) noexcept
{
    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
    case UiLayoutAnchor::TopCenter:
    case UiLayoutAnchor::TopRight:
        return margin.top;
    case UiLayoutAnchor::CenterLeft:
    case UiLayoutAnchor::Center:
    case UiLayoutAnchor::CenterRight:
        return margin.top - margin.bottom;
    case UiLayoutAnchor::BottomLeft:
    case UiLayoutAnchor::BottomCenter:
    case UiLayoutAnchor::BottomRight:
        return -margin.bottom;
    default:
        return 0.0f;
    }
}

[[nodiscard]] elysia::core::Rect anchored_rect(
    const elysia::core::Rect& bounds,
    const elysia::core::Vector2& size,
    UiLayoutAnchor anchor,
    const UiLayoutMargin& margin
) noexcept
{
    const elysia::core::Vector2 clamped_size = clamp_size(size);
    elysia::core::Rect rect = elysia::core::Rect::from_center(bounds.center(),clamped_size);

    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
        rect.set_position(bounds.top_left());
        break;
    case UiLayoutAnchor::TopCenter:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f,bounds.top()));
        break;
    case UiLayoutAnchor::TopRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x,bounds.top()));
        break;
    case UiLayoutAnchor::CenterLeft:
        rect.set_position(elysia::core::Vector2(bounds.left(),bounds.center().y - clamped_size.y * 0.5f));
        break;
    case UiLayoutAnchor::Center:
        rect.set_center(bounds.center());
        break;
    case UiLayoutAnchor::CenterRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x,bounds.center().y - clamped_size.y * 0.5f));
        break;
    case UiLayoutAnchor::BottomLeft:
        rect.set_position(elysia::core::Vector2(bounds.left(),bounds.bottom() - clamped_size.y));
        break;
    case UiLayoutAnchor::BottomCenter:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f,bounds.bottom() - clamped_size.y));
        break;
    case UiLayoutAnchor::BottomRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x,bounds.bottom() - clamped_size.y));
        break;
    }

    rect.set_position(elysia::core::Vector2(
        rect.x() + horizontal_margin_offset(anchor,margin),
        rect.y() + vertical_margin_offset(anchor,margin)
    ));
    return rect;
}
}

UiPanel::UiPanel(const elysia::core::Rect& rect,int order) noexcept
    : UiContainer(rect,order) {}

UiPanel::UiPanel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiContainer(position,size,order) {}

UiPanel::UiPanel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiContainer(center,size,from_center,order) {}

void UiPanel::reset() noexcept
{
    UiContainer::reset();
    _draw_background = false;
    _draw_border = false;
    _background_color = elysia::core::colors::cobalt_blue;
    _border_color = elysia::core::colors::sky_blue;
}

void UiPanel::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const elysia::core::Rect& rect = screen_rect();
    if (_draw_background && !rect.is_empty())
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(rect,apply_opacity(_background_color)));
    submit_child_render_commands(out_commands);
    if (_draw_border && !rect.is_empty())
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(rect,apply_opacity(_border_color)));
}

void UiPanel::set_draw_background(bool draw_background) noexcept
{
    _draw_background = draw_background;
}

bool UiPanel::draws_background() const noexcept
{
    return _draw_background;
}

void UiPanel::set_draw_border(bool draw_border) noexcept
{
    _draw_border = draw_border;
}

bool UiPanel::draws_border() const noexcept
{
    return _draw_border;
}

void UiPanel::set_background_color(elysia::core::Color color) noexcept
{
    _background_color = color;
}

elysia::core::Color UiPanel::background_color() const noexcept
{
    return _background_color;
}

void UiPanel::set_border_color(elysia::core::Color color) noexcept
{
    _border_color = color;
}

elysia::core::Color UiPanel::border_color() const noexcept
{
    return _border_color;
}

void UiPanel::rebuild_layout()
{
    const elysia::core::Rect bounds = content_rect();
    for (ChildEntry& entry : children())
    {
        if (!entry.element)
            continue;

        const elysia::core::Vector2 size = entry.layout._use_size_override
            ? clamp_size(entry.layout._size_override)
            : clamp_size(entry.element->size());
        entry.element->set_screen_rect(anchored_rect(bounds,size,entry.layout._anchor,entry.layout._margin));
    }
}
}
