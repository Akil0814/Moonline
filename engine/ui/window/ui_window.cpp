#include "ui_window.h"

#include "../layout/ui_anchor_layout.h"
#include "../../core/render/render_command.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
[[nodiscard]] bool is_navigation_action(UiAction action) noexcept
{
    switch (action)
    {
    case UiAction::NavigateLeft:
    case UiAction::NavigateRight:
    case UiAction::NavigateUp:
    case UiAction::NavigateDown:
        return true;
    default:
        return false;
    }
}

[[nodiscard]] std::size_t child_count_of(const UiElement& element) noexcept
{
    if (const auto* child_host = dynamic_cast<const UiChildHost*>(&element))
        return child_host->child_count();
    return 0;
}

[[nodiscard]] const UiElement* child_at_of(const UiElement& element,std::size_t index) noexcept
{
    if (const auto* child_host = dynamic_cast<const UiChildHost*>(&element))
        return child_host->child_at(index);
    return nullptr;
}

void collect_live_controls(const UiElement& element,std::vector<const UiControl*>& out_controls)
{
    if (element.is_destroyed())
        return;
    if (const auto* control = dynamic_cast<const UiControl*>(&element))
        out_controls.push_back(control);
    const std::size_t count = child_count_of(element);
    for (std::size_t index = 0; index < count; ++index)
    {
        const UiElement* child = child_at_of(element,index);
        if (child)
            collect_live_controls(*child,out_controls);
    }
}

[[nodiscard]] bool contains_control(const std::vector<const UiControl*>& controls,const UiControl* control) noexcept
{
    return std::find(controls.begin(),controls.end(),control) != controls.end();
}
}

UiWindow::UiWindow(const elysia::core::Rect& rect,int order) noexcept : UiChildHost(rect,order) {}
UiWindow::UiWindow(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept : UiChildHost(position,size,order) {}
UiWindow::UiWindow(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag tag,int order) noexcept : UiChildHost(center,size,tag,order) {}

void UiWindow::reset() noexcept
{
    UiChildHost::reset();
    _focus_entries.clear();
    _focused_target = nullptr;
    _draw_background = false;
    _draw_border = false;
    _background_color = elysia::core::colors::cobalt_blue;
    _border_color = elysia::core::colors::sky_blue;
    _hover_focus_enabled = true;
    _on_cancel = {};
}

void UiWindow::set_draw_background(bool draw_background) noexcept
{
    _draw_background = draw_background;
}

bool UiWindow::draws_background() const noexcept
{
    return _draw_background;
}

void UiWindow::set_draw_border(bool draw_border) noexcept
{
    _draw_border = draw_border;
}

bool UiWindow::draws_border() const noexcept
{
    return _draw_border;
}

void UiWindow::set_background_color(elysia::core::Color color) noexcept
{
    _background_color = color;
}

elysia::core::Color UiWindow::background_color() const noexcept
{
    return _background_color;
}

void UiWindow::set_border_color(elysia::core::Color color) noexcept
{
    _border_color = color;
}

elysia::core::Color UiWindow::border_color() const noexcept
{
    return _border_color;
}

void UiWindow::set_hover_focus_enabled(bool enabled) noexcept
{
    _hover_focus_enabled = enabled;
}

bool UiWindow::hover_focus_enabled() const noexcept
{
    return _hover_focus_enabled;
}

void UiWindow::set_on_cancel(UiWindowCancelCallback on_cancel)
{
    _on_cancel = std::move(on_cancel);
}

void UiWindow::register_focus_target(UiControl& control,const UiWindowFocusOptions& options)
{
    auto found = std::find_if(_focus_entries.begin(),_focus_entries.end(),[&control](const FocusEntry& entry)
    {
        return entry.control == &control;
    });
    if (found != _focus_entries.end())
        found->options = options;
    else
        _focus_entries.push_back(FocusEntry{ &control,options });
    ensure_valid_focus();
    apply_focus_state();
}

void UiWindow::unregister_focus_target(UiControl& control)
{
    _focus_entries.erase(std::remove_if(_focus_entries.begin(),_focus_entries.end(),[&control](const FocusEntry& entry)
    {
        return entry.control == &control;
    }),_focus_entries.end());
    if (_focused_target == &control)
        _focused_target = nullptr;
    ensure_valid_focus();
    apply_focus_state();
}

void UiWindow::set_focus_neighbors(UiControl& control,const UiWindowFocusNeighbors& neighbors)
{
    auto found = std::find_if(_focus_entries.begin(),_focus_entries.end(),[&control](const FocusEntry& entry)
    {
        return entry.control == &control;
    });
    if (found == _focus_entries.end())
    {
        register_focus_target(control,UiWindowFocusOptions{ .neighbors = neighbors });
        return;
    }
    found->options.neighbors = neighbors;
}

void UiWindow::set_focused_target(UiControl* control)
{
    (void)set_focused_target_internal(control);
    ensure_valid_focus();
    apply_focus_state();
}

UiControl* UiWindow::focused_target() const noexcept
{
    return _focused_target;
}

bool UiWindow::focus_first_available()
{
    for (const FocusEntry& entry : _focus_entries)
    {
        if (is_control_usable(entry.control))
        {
            _focused_target = entry.control;
            apply_focus_state();
            return true;
        }
    }
    _focused_target = nullptr;
    apply_focus_state();
    return false;
}

void UiWindow::update(double delta)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_targets();
    ensure_valid_focus();
    apply_focus_state();
    update_child_objects(delta);
    cleanup_destroyed_children();
    prune_focus_targets();
    ensure_valid_focus();
    apply_focus_state();
}

void UiWindow::on_ui_input_frame(const UiInputFrame& input)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_targets();
    ensure_valid_focus();
    apply_focus_state();
    dispatch_frame_to_children(input);
    cleanup_destroyed_children();
    prune_focus_targets();
    ensure_valid_focus();
    apply_focus_state();
}

bool UiWindow::on_ui_input_event(const UiInputEvent& event)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_targets();
    ensure_valid_focus();
    apply_focus_state();

    bool handled = false;

    if (event.type == UiInputEventType::ActionPressed && event.action == UiAction::Cancel)
    {
        if (_on_cancel)
        {
            _on_cancel();
            handled = true;
        }
    }
    else if (event.type == UiInputEventType::ActionPressed && event.action == UiAction::Confirm)
    {
        if (is_control_usable(_focused_target))
        {
            if (auto* receiver = dynamic_cast<UiInputEventReceiver*>(_focused_target))
                handled = receiver->on_ui_input_event(event);
        }
    }
    else if (is_navigation_action(event.action) && event.type == UiInputEventType::ActionPressed)
    {
        if (is_control_usable(_focused_target))
        {
            if (auto* receiver = dynamic_cast<UiInputEventReceiver*>(_focused_target))
                handled = receiver->on_ui_input_event(event);
        }
        if (!handled && _focused_target)
        {
            UiControl* neighbor = find_neighbor(*_focused_target,event.action);
            if (neighbor)
            {
                _focused_target = neighbor;
                handled = true;
            }
        }
    }
    else
    {
        if (event.type == UiInputEventType::MouseMoved && _hover_focus_enabled)
        {
            if (UiControl* hovered = find_registered_target_at(event.mouse_x,event.mouse_y))
                (void)set_focused_target_internal(hovered);
        }
        else if (event.type == UiInputEventType::PointerPressed && event.control == elysia::input::RawInputControl::MouseLeft)
        {
            if (UiControl* hovered = find_registered_target_at(event.mouse_x,event.mouse_y))
                (void)set_focused_target_internal(hovered);
        }
        handled = dispatch_input_to_children(event);
    }

    cleanup_destroyed_children();
    prune_focus_targets();
    ensure_valid_focus();
    apply_focus_state();
    return handled;
}

void UiWindow::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;
    auto* self = const_cast<UiWindow*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    self->prune_focus_targets();
    self->ensure_valid_focus();
    self->apply_focus_state();

    if (_draw_background)
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(screen_rect(),apply_opacity(_background_color)));
    submit_child_render_commands(out_commands);
    if (_draw_border)
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(screen_rect(),apply_opacity(_border_color)));
}

void UiWindow::rebuild_layout()
{
    layout::layout_anchored_children(children(),content_rect());
}

void UiWindow::prune_focus_targets()
{
    std::vector<const UiControl*> live_controls;
    live_controls.reserve(_focus_entries.size());
    for (const ChildEntry& child : children())
    {
        if (child.element)
            collect_live_controls(*child.element,live_controls);
    }
    _focus_entries.erase(std::remove_if(_focus_entries.begin(),_focus_entries.end(),[&live_controls](const FocusEntry& entry)
    {
        return !entry.control || entry.control->is_destroyed() || !contains_control(live_controls,entry.control);
    }),_focus_entries.end());
    if (_focused_target && (_focused_target->is_destroyed() || !contains_control(live_controls,_focused_target)))
        _focused_target = nullptr;
}

void UiWindow::ensure_valid_focus()
{
    if (_focused_target && is_registered_focus_target(*_focused_target) && is_control_usable(_focused_target))
        return;
    focus_first_available();
}

void UiWindow::apply_focus_state()
{
    for (const FocusEntry& entry : _focus_entries)
    {
        if (!entry.control)
            continue;
        entry.control->set_focused(entry.control == _focused_target && is_control_usable(entry.control));
    }
}

UiControl* UiWindow::find_registered_target_at(int mouse_x,int mouse_y) const
{
    const elysia::core::Vector2 pointer(static_cast<float>(mouse_x),static_cast<float>(mouse_y));
    auto find_in_element = [&](const auto& self,const UiElement& element) -> UiControl*
    {
        const std::size_t count = child_count_of(element);
        for (std::size_t offset = 0; offset < count; ++offset)
        {
            const UiElement* child = child_at_of(element,count - 1 - offset);
            if (!child || child->is_destroyed() || !child->is_visible() || !child->is_active())
                continue;
            if (UiControl* nested = self(self,*child))
                return nested;
        }
        const auto* control = dynamic_cast<const UiControl*>(&element);
        if (!control || !is_registered_focus_target(*control) || !is_control_usable(control))
            return nullptr;
        return control->screen_rect().contains(pointer) ? const_cast<UiControl*>(control) : nullptr;
    };

    for (std::size_t offset = 0; offset < child_count(); ++offset)
    {
        const UiElement* child = child_at(child_count() - 1 - offset);
        if (!child || child->is_destroyed() || !child->is_visible() || !child->is_active())
            continue;
        if (UiControl* found = find_in_element(find_in_element,*child))
            return found;
    }
    return nullptr;
}

UiControl* UiWindow::find_neighbor(const UiControl& control,UiAction action) const
{
    auto found = std::find_if(_focus_entries.begin(),_focus_entries.end(),[&control](const FocusEntry& entry)
    {
        return entry.control == &control;
    });
    if (found == _focus_entries.end())
        return nullptr;

    UiControl* candidate = nullptr;
    switch (action)
    {
    case UiAction::NavigateLeft:
        candidate = found->options.neighbors.left;
        break;
    case UiAction::NavigateRight:
        candidate = found->options.neighbors.right;
        break;
    case UiAction::NavigateUp:
        candidate = found->options.neighbors.up;
        break;
    case UiAction::NavigateDown:
        candidate = found->options.neighbors.down;
        break;
    default:
        break;
    }
    return candidate && is_registered_focus_target(*candidate) && is_control_usable(candidate) ? candidate : nullptr;
}

bool UiWindow::is_registered_focus_target(const UiControl& control) const noexcept
{
    return std::any_of(_focus_entries.begin(),_focus_entries.end(),[&control](const FocusEntry& entry)
    {
        return entry.control == &control;
    });
}

bool UiWindow::set_focused_target_internal(UiControl* control) noexcept
{
    if (!control)
    {
        _focused_target = nullptr;
        return true;
    }
    if (!is_registered_focus_target(*control) || !is_control_usable(control))
        return false;
    _focused_target = control;
    return true;
}

bool UiWindow::is_control_usable(const UiControl* control) noexcept
{
    return control && !control->is_destroyed() && control->is_active() && control->is_visible() && control->is_enabled();
}
}
