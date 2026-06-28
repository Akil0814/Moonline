#include "ui_panel.h"
#include "ui_container_shared_utils.h"

namespace elysia::ui
{
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
            ? container_utils::clamp_size(entry.layout._size_override)
            : container_utils::clamp_size(entry.element->size());
        entry.element->set_screen_rect(container_utils::anchored_rect(bounds,size,entry.layout._anchor,entry.layout._margin));
    }
}
}
