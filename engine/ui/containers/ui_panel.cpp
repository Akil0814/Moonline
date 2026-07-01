#include "ui_panel.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
[[nodiscard]] UiPanelInsertDirection opposite_direction(UiPanelInsertDirection direction) noexcept
{
    switch (direction)
    {
    case UiPanelInsertDirection::Up:
        return UiPanelInsertDirection::Down;
    case UiPanelInsertDirection::Down:
        return UiPanelInsertDirection::Up;
    case UiPanelInsertDirection::Left:
        return UiPanelInsertDirection::Right;
    case UiPanelInsertDirection::Right:
    default:
        return UiPanelInsertDirection::Left;
    }
}

std::optional<UiControl*>& neighbor_slot(UiFocusNeighbors& neighbors,UiPanelInsertDirection direction) noexcept
{
    switch (direction)
    {
    case UiPanelInsertDirection::Up:
        return neighbors.up;
    case UiPanelInsertDirection::Down:
        return neighbors.down;
    case UiPanelInsertDirection::Left:
        return neighbors.left;
    case UiPanelInsertDirection::Right:
    default:
        return neighbors.right;
    }
}
}

UiPanel::UiPanel(const elysia::core::Rect& rect,int order) noexcept
    : UiControlFocusScopeHost(rect,order) {}

UiPanel::UiPanel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiControlFocusScopeHost(position,size,order) {}

UiPanel::UiPanel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiControlFocusScopeHost(center,size,from_center,order) {}

void UiPanel::reset() noexcept
{
    UiControlFocusScopeHost::reset();
    _draw_background = false;
    _draw_border = false;
    _background_color = elysia::core::colors::cobalt_blue;
    _border_color = elysia::core::colors::sky_blue;
    _focus_links.clear();
    _last_focusable = nullptr;
}

void UiPanel::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    auto* self = const_cast<UiPanel*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    self->refresh_focus_registry();
    self->ensure_valid_focus();
    self->apply_focus_state();

    const elysia::core::Rect& rect = screen_rect();
    if (_draw_background && !rect.is_empty())
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(rect,apply_opacity(_background_color)));
    submit_child_render_commands(out_commands);
    if (_draw_border && !rect.is_empty())
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(rect,apply_opacity(_border_color)));
}

void UiPanel::add_child(std::unique_ptr<UiElement> child,UiPanelInsertDirection direction)
{
    (void)insert_panel_child(std::move(child),direction);
}

UiElement* UiPanel::add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options)
{
    (void)options;
    return insert_panel_child(std::move(child),UiPanelInsertDirection::Down);
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
}

void UiPanel::rebuild_focus_registry()
{
    prune_panel_links();

    std::vector<FocusEntry> entries;
    const std::vector<UiControl*> controls = direct_focusable_children();
    entries.reserve(controls.size());
    for (UiControl* control : controls)
    {
        FocusLink& link = ensure_link(*control);
        entries.push_back(FocusEntry{ control,link.neighbors });
    }
    set_focus_entries(std::move(entries));
}

UiElement* UiPanel::insert_panel_child(std::unique_ptr<UiElement> child,UiPanelInsertDirection direction)
{
    if (!child)
        return nullptr;

    UiControl* control = dynamic_cast<UiControl*>(child.get());
    UiElement* added = UiChildHost::add_child(std::move(child));
    if (!added || !control)
        return added;

    prune_panel_links();
    FocusLink& current = ensure_link(*control);
    if (_last_focusable && _last_focusable != control)
    {
        FocusLink& previous = ensure_link(*_last_focusable);
        neighbor_slot(previous.neighbors,direction) = control;
        neighbor_slot(current.neighbors,opposite_direction(direction)) = _last_focusable;
    }
    _last_focusable = control;
    return added;
}

void UiPanel::prune_panel_links()
{
    const std::vector<UiControl*> live_controls = direct_focusable_children();
    auto is_live = [&live_controls](const UiControl* control)
    {
        return control && std::find(live_controls.begin(),live_controls.end(),control) != live_controls.end();
    };

    _focus_links.erase(std::remove_if(_focus_links.begin(),_focus_links.end(),[&](const FocusLink& link)
    {
        return !is_live(link.control);
    }),_focus_links.end());

    for (FocusLink& link : _focus_links)
    {
        if (link.neighbors.up.has_value() && !is_live(*link.neighbors.up))
            link.neighbors.up.reset();
        if (link.neighbors.down.has_value() && !is_live(*link.neighbors.down))
            link.neighbors.down.reset();
        if (link.neighbors.left.has_value() && !is_live(*link.neighbors.left))
            link.neighbors.left.reset();
        if (link.neighbors.right.has_value() && !is_live(*link.neighbors.right))
            link.neighbors.right.reset();
    }

    if (!is_live(_last_focusable))
        _last_focusable = live_controls.empty() ? nullptr : live_controls.back();
}

UiPanel::FocusLink& UiPanel::ensure_link(UiControl& control)
{
    if (FocusLink* link = find_link(control))
        return *link;

    _focus_links.push_back(FocusLink{ &control,{} });
    return _focus_links.back();
}

UiPanel::FocusLink* UiPanel::find_link(UiControl& control) noexcept
{
    auto found = std::find_if(_focus_links.begin(),_focus_links.end(),[&control](const FocusLink& link)
    {
        return link.control == &control;
    });
    return found != _focus_links.end() ? &(*found) : nullptr;
}

const UiPanel::FocusLink* UiPanel::find_link(const UiControl& control) const noexcept
{
    auto found = std::find_if(_focus_links.begin(),_focus_links.end(),[&control](const FocusLink& link)
    {
        return link.control == &control;
    });
    return found != _focus_links.end() ? &(*found) : nullptr;
}
}

