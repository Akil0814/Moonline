#include "ui_panel.h"

#include "../layout/ui_layout_geometry.h"
#include "../style/ui_style_defaults.h"

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

[[nodiscard]] bool is_zero_margin(const UiLayoutMargin& margin) noexcept
{
    return margin.left == 0.0f
        && margin.top == 0.0f
        && margin.right == 0.0f
        && margin.bottom == 0.0f;
}

[[nodiscard]] bool uses_panel_anchor_layout(const UiLayoutChildOptions& options) noexcept
{
    return options._anchor != UiLayoutAnchor::TopLeft
        || !is_zero_margin(options._margin)
        || options._use_size_override
        || options._use_custom_cross_align
        || options._fill_cross_axis;
}
}

UiPanel::UiPanel(const elysia::core::Rect& rect,int order) noexcept
    : UiControlFocusScopeHost(rect,order)
{
    reset();
}

UiPanel::UiPanel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiControlFocusScopeHost(position,size,order)
{
    reset();
}

UiPanel::UiPanel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiControlFocusScopeHost(center,size,from_center,order)
{
    reset();
}

void UiPanel::reset() noexcept
{
    UiControlFocusScopeHost::reset();
    _style = UiStyleDefaults::panel();
    _focus_links.clear();
    _last_focusable = nullptr;
    _last_child_layout_origin = elysia::core::Vector2::zero();
    _has_child_layout_origin = false;
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
    if (_style.draw_background && !rect.is_empty())
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(rect,apply_opacity(_style.background)));
    submit_child_render_commands(out_commands);
    if (_style.draw_border && !rect.is_empty())
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(rect,apply_opacity(_style.border)));
}

elysia::core::Vector2 UiPanel::content_extent() const noexcept
{
    return size();
}

void UiPanel::add_child(std::unique_ptr<UiElement> child,UiPanelInsertDirection direction)
{
    (void)insert_panel_child(std::move(child),direction);
}

UiElement* UiPanel::add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options)
{
    if (!uses_panel_anchor_layout(options))
        return insert_panel_child(std::move(child),UiPanelInsertDirection::Down);

    return UiChildHost::add_child(std::move(child),options);
}

void UiPanel::set_style(const UiPanelStyle& style) noexcept
{
    _style = style;
}

const UiPanelStyle& UiPanel::style() const noexcept
{
    return _style;
}

void UiPanel::set_draw_background(bool draw_background) noexcept
{
    _style.draw_background = draw_background;
}

bool UiPanel::draws_background() const noexcept
{
    return _style.draw_background;
}

void UiPanel::set_draw_border(bool draw_border) noexcept
{
    _style.draw_border = draw_border;
}

bool UiPanel::draws_border() const noexcept
{
    return _style.draw_border;
}

void UiPanel::set_background_color(elysia::core::Color color) noexcept
{
    _style.background = color;
}

elysia::core::Color UiPanel::background_color() const noexcept
{
    return _style.background;
}

void UiPanel::set_border_color(elysia::core::Color color) noexcept
{
    _style.border = color;
}

elysia::core::Color UiPanel::border_color() const noexcept
{
    return _style.border;
}

void UiPanel::rebuild_layout()
{
    const elysia::core::Rect bounds = content_rect();
    const elysia::core::Vector2 current_origin = position();
    const elysia::core::Vector2 delta = _has_child_layout_origin
        ? (current_origin - _last_child_layout_origin)
        : current_origin;

    for (ChildEntry& entry : children())
    {
        if (!entry.element)
            continue;

        if (uses_panel_anchor_layout(entry.layout))
        {
            const elysia::core::Vector2 child_size = entry.layout._use_size_override
                ? layout::clamp_size(entry.layout._size_override)
                : layout::clamp_size(entry.element->size());
            entry.element->set_screen_rect(layout::anchored_rect(bounds,child_size,entry.layout._anchor,entry.layout._margin));
            continue;
        }

        if (!delta.is_zero())
            entry.element->set_position(entry.element->position() + delta);
    }

    _last_child_layout_origin = current_origin;
    _has_child_layout_origin = true;
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
    if (added && _has_child_layout_origin)
        added->set_position(added->position() + _last_child_layout_origin);
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

