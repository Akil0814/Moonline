#include "ui_window.h"

#include "../focus/ui_focus_scope_utils.h"
#include "../layout/ui_anchor_layout.h"
#include "../../core/render/render_command.h"

#include <algorithm>

namespace elysia::ui
{
UiWindow::UiWindow(const elysia::core::Rect& rect,int order) noexcept : UiChildHost(rect,order) {}
UiWindow::UiWindow(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept : UiChildHost(position,size,order) {}
UiWindow::UiWindow(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag tag,int order) noexcept : UiChildHost(center,size,tag,order) {}

void UiWindow::reset() noexcept
{
    UiChildHost::reset();
    _scope_entries.clear();
    _focused_scope = nullptr;
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

void UiWindow::register_focus_scope(UiFocusScope& scope,const UiFocusScopeNeighbors& neighbors)
{
    auto found = std::find_if(_scope_entries.begin(),_scope_entries.end(),[&scope](const ScopeEntry& entry)
    {
        return entry.scope == &scope;
    });
    if (found != _scope_entries.end())
        found->neighbors = neighbors;
    else
        _scope_entries.push_back(ScopeEntry{ &scope,neighbors });
    ensure_valid_scope_focus();
    apply_scope_focus();
}

void UiWindow::unregister_focus_scope(UiFocusScope& scope)
{
    _scope_entries.erase(std::remove_if(_scope_entries.begin(),_scope_entries.end(),[&scope](const ScopeEntry& entry)
    {
        return entry.scope == &scope;
    }),_scope_entries.end());
    if (_focused_scope == &scope)
        _focused_scope = nullptr;
    ensure_valid_scope_focus();
    apply_scope_focus();
}

void UiWindow::set_scope_neighbors(UiFocusScope& scope,const UiFocusScopeNeighbors& neighbors)
{
    auto found = std::find_if(_scope_entries.begin(),_scope_entries.end(),[&scope](const ScopeEntry& entry)
    {
        return entry.scope == &scope;
    });
    if (found == _scope_entries.end())
    {
        register_focus_scope(scope,neighbors);
        return;
    }
    found->neighbors = neighbors;
}

void UiWindow::set_focused_scope(UiFocusScope* scope)
{
    (void)set_focused_scope_internal(scope);
    ensure_valid_scope_focus();
    apply_scope_focus();
}

UiFocusScope* UiWindow::focused_scope() const noexcept
{
    return _focused_scope;
}

bool UiWindow::focus_first_available_scope()
{
    prune_focus_scopes();
    for (const ScopeEntry& entry : _scope_entries)
    {
        if (is_scope_usable(entry.scope))
        {
            _focused_scope = entry.scope;
            if (!_focused_scope->focused_target())
                (void)_focused_scope->focus_first_available();
            apply_scope_focus();
            return true;
        }
    }
    _focused_scope = nullptr;
    apply_scope_focus();
    return false;
}

void UiWindow::update(double delta)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
    update_child_objects(delta);
    cleanup_destroyed_children();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
}

void UiWindow::on_ui_input_frame(const UiInputFrame& input)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
    dispatch_frame_to_children(input);
    cleanup_destroyed_children();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
}

bool UiWindow::on_ui_input_event(const UiInputEvent& event)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();

    bool handled = false;

    if (event.type == UiInputEventType::ActionPressed && event.action == UiAction::Cancel)
    {
        if (_on_cancel)
        {
            _on_cancel();
            handled = true;
        }
    }
    else
    {
        UiFocusScope* pointer_scope = nullptr;
        if (event.type == UiInputEventType::MouseMoved
            || event.type == UiInputEventType::PointerPressed
            || event.type == UiInputEventType::PointerReleased
            || event.type == UiInputEventType::MouseWheel)
        {
            pointer_scope = find_registered_scope_at(event.mouse_x,event.mouse_y);
        }

        if (event.type == UiInputEventType::MouseMoved && _hover_focus_enabled && pointer_scope)
            (void)set_focused_scope_internal(pointer_scope);
        else if (event.type == UiInputEventType::PointerPressed
            && event.device == elysia::input::InputDevice::Mouse
            && event.control == elysia::input::RawInputControl::MouseLeft
            && pointer_scope)
            (void)set_focused_scope_internal(pointer_scope);

        if (event.type == UiInputEventType::ActionPressed && is_navigation_action(event.action))
        {
            handled = dispatch_to_scope(_focused_scope,event);
            if (!handled && _focused_scope)
            {
                if (UiFocusScope* neighbor = find_neighbor(*_focused_scope,event.action))
                {
                    _focused_scope = neighbor;
                    if (!_focused_scope->focused_target())
                        (void)_focused_scope->focus_first_available();
                    handled = true;
                }
            }
        }
        else if ((event.action == UiAction::Confirm)
            && (event.type == UiInputEventType::ActionPressed || event.type == UiInputEventType::ActionReleased))
        {
            handled = dispatch_to_scope(_focused_scope,event);
        }
        else
        {
            UiFocusScope* event_scope = pointer_scope ? pointer_scope : _focused_scope;
            if (event_scope)
                handled = dispatch_to_scope(event_scope,event);
            if (!handled)
                handled = dispatch_input_to_children(event);
        }
    }

    cleanup_destroyed_children();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
    return handled;
}

void UiWindow::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;
    auto* self = const_cast<UiWindow*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    self->prune_focus_scopes();
    self->ensure_valid_scope_focus();
    self->apply_scope_focus();

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

void UiWindow::prune_focus_scopes()
{
    std::vector<const UiFocusScope*> live_scopes;
    live_scopes.reserve(_scope_entries.size());
    collect_live_scopes(*this,live_scopes);
    _scope_entries.erase(std::remove_if(_scope_entries.begin(),_scope_entries.end(),[&live_scopes](const ScopeEntry& entry)
    {
        return !entry.scope || !contains_scope(live_scopes,entry.scope);
    }),_scope_entries.end());
    if (_focused_scope && !contains_scope(live_scopes,_focused_scope))
        _focused_scope = nullptr;
}

void UiWindow::ensure_valid_scope_focus()
{
    if (_focused_scope && is_registered_scope(*_focused_scope) && is_scope_usable(_focused_scope))
        return;
    (void)focus_first_available_scope();
}

void UiWindow::apply_scope_focus()
{
    for (const ScopeEntry& entry : _scope_entries)
    {
        if (!entry.scope)
            continue;
        entry.scope->set_scope_focused(entry.scope == _focused_scope && is_scope_usable(entry.scope));
    }
}

UiFocusScope* UiWindow::find_registered_scope_at(int mouse_x,int mouse_y) const
{
    for (std::size_t index = _scope_entries.size(); index > 0; --index)
    {
        UiFocusScope* scope = _scope_entries[index - 1].scope;
        if (!is_scope_usable(scope))
            continue;
        if (scope->contains_focus_point(mouse_x,mouse_y))
            return scope;
    }
    return nullptr;
}

UiFocusScope* UiWindow::find_neighbor(const UiFocusScope& scope,UiAction action) const
{
    auto found = std::find_if(_scope_entries.begin(),_scope_entries.end(),[&scope](const ScopeEntry& entry)
    {
        return entry.scope == &scope;
    });
    if (found == _scope_entries.end())
        return nullptr;

    UiFocusScope* candidate = nullptr;
    switch (action)
    {
    case UiAction::NavigateLeft:
        candidate = found->neighbors.left;
        break;
    case UiAction::NavigateRight:
        candidate = found->neighbors.right;
        break;
    case UiAction::NavigateUp:
        candidate = found->neighbors.up;
        break;
    case UiAction::NavigateDown:
        candidate = found->neighbors.down;
        break;
    default:
        break;
    }
    return candidate && is_registered_scope(*candidate) && is_scope_usable(candidate) ? candidate : nullptr;
}

bool UiWindow::is_registered_scope(const UiFocusScope& scope) const noexcept
{
    return std::any_of(_scope_entries.begin(),_scope_entries.end(),[&scope](const ScopeEntry& entry)
    {
        return entry.scope == &scope;
    });
}

bool UiWindow::set_focused_scope_internal(UiFocusScope* scope) noexcept
{
    if (!scope)
    {
        _focused_scope = nullptr;
        return true;
    }
    if (!is_registered_scope(*scope) || !is_scope_usable(scope))
        return false;
    _focused_scope = scope;
    if (!_focused_scope->focused_target())
        (void)_focused_scope->focus_first_available();
    return true;
}

bool UiWindow::dispatch_to_scope(UiFocusScope* scope,const UiInputEvent& event) const
{
    if (!is_scope_usable(scope))
        return false;
    if (auto* receiver = dynamic_cast<UiInputEventReceiver*>(&scope->focus_scope_element()))
        return receiver->on_ui_input_event(event);
    return false;
}

bool UiWindow::is_scope_usable(const UiFocusScope* scope) noexcept
{
    if (!scope)
        return false;
    const UiElement& element = scope->focus_scope_element();
    return !element.is_destroyed() && element.is_active() && element.is_visible() && scope->has_focusable_target();
}
}
