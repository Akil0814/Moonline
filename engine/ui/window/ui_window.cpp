#include "ui_window.h"

#include "../style/ui_style_defaults.h"
#include "../style/ui_theme.h"
#include "../focus/ui_focus_scope_utils.h"
#include "../focus/ui_control_focus_scope_host.h"
#include "../layout/ui_anchor_layout.h"
#include "../layout/ui_layout_geometry.h"
#include "../../core/render/render_command.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
[[nodiscard]] bool contains_child_element(const UiElement& root,const UiElement& target) noexcept
{
    if (&root == &target)
        return true;

    const auto* child_host = dynamic_cast<const UiChildHost*>(&root);
    if (!child_host)
        return false;

    for (std::size_t index = 0; index < child_host->child_count(); ++index)
    {
        const UiElement* child = child_host->child_at(index);
        if (child && contains_child_element(*child,target))
            return true;
    }
    return false;
}

[[nodiscard]] bool is_primary_mouse_press(const UiInputEvent& event) noexcept
{
    return event.type == UiInputEventType::PointerPressed
        && event.device == elysia::input::InputDevice::Mouse
        && event.control == elysia::input::RawInputControl::MouseLeft;
}
}

UiWindow::UiWindow(const elysia::core::Rect& rect,int order) noexcept : UiChildHost(rect,order)
{
    reset();
}
UiWindow::UiWindow(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept : UiChildHost(position,size,order)
{
    reset();
}
UiWindow::UiWindow(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag tag,int order) noexcept : UiChildHost(center,size,tag,order)
{
    reset();
}

void UiWindow::reset() noexcept
{
    UiChildHost::reset();
    _scope_entries.clear();
    _overlay_entries.clear();
    _focused_scope = nullptr;
    _last_focused_scope = nullptr;
    _style_state.reset(UiStyleDefaults::window());
    _hover_focus_enabled = true;
    _focus_input_device = elysia::input::InputDevice::Unknown;
    _on_cancel = {};
}

void UiWindow::set_style(const UiWindowStyle& style) noexcept
{
    _style_state.set_style_override(style);
}

const UiWindowStyle& UiWindow::style() const noexcept
{
    return _style_state.effective_style();
}

bool UiWindow::has_style_override() const noexcept
{
    return _style_state.has_style_override();
}

void UiWindow::clear_style_override() noexcept
{
    _style_state.clear_style_override();
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
    if (_last_focused_scope == &scope)
        _last_focused_scope = nullptr;
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
    cleanup_destroyed_children();
    update_layout_if_dirty();
    prune_focus_scopes();
    for (const ScopeEntry& entry : _scope_entries)
    {
        if (is_scope_usable(entry.scope))
        {
            (void)set_focused_scope_internal(entry.scope);
            apply_scope_focus();
            return true;
        }
    }
    _focused_scope = nullptr;
    apply_scope_focus();
    return false;
}

void UiWindow::register_overlay(UiElement& element,UiOverlayOptions options)
{
    if (OverlayEntry* entry = find_overlay(element))
    {
        const bool was_open = entry->options.open;
        entry->options = options;
        if (!options.open)
            entry->restore_focus_scope = nullptr;
        element.set_order(options.order);
        sync_overlay_visibility(*entry);
        mark_layout_dirty();
        if (options.open)
        {
            if (!was_open)
                remember_overlay_restore_focus(*entry);
            update_layout_if_dirty();
            (void)focus_overlay(*entry);
        }
    }
    else
    {
        _overlay_entries.push_back(OverlayEntry{ &element,options });
        OverlayEntry& added = _overlay_entries.back();
        element.set_order(options.order);
        sync_overlay_visibility(added);
        mark_layout_dirty();
        if (options.open)
        {
            remember_overlay_restore_focus(added);
            update_layout_if_dirty();
            (void)focus_overlay(added);
        }
    }

    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
}

void UiWindow::unregister_overlay(UiElement& element)
{
    _overlay_entries.erase(std::remove_if(_overlay_entries.begin(),_overlay_entries.end(),[&element](const OverlayEntry& entry)
    {
        return entry.element == &element;
    }),_overlay_entries.end());
}

void UiWindow::set_overlay_open(UiElement& element,bool open)
{
    OverlayEntry* entry = find_overlay(element);
    if (!entry)
        return;

    const bool was_open = entry->options.open;
    if (open && !was_open)
        remember_overlay_restore_focus(*entry);

    entry->options.open = open;
    sync_overlay_visibility(*entry);
    mark_layout_dirty();
    update_layout_if_dirty();

    if (open)
    {
        (void)focus_overlay(*entry);
    }
    else if (!restore_focus_after_overlay_close(*entry))
    {
        if (OverlayEntry* active = active_overlay())
            (void)focus_overlay(*active);
    }

    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
}

bool UiWindow::is_overlay_open(const UiElement& element) const noexcept
{
    const OverlayEntry* entry = find_overlay(element);
    return entry && entry->options.open;
}

UiOverlayOptions* UiWindow::overlay_options(UiElement& element) noexcept
{
    OverlayEntry* entry = find_overlay(element);
    return entry ? &entry->options : nullptr;
}

void UiWindow::update(double delta)
{
    cleanup_destroyed_children();
    prune_overlays();
    sync_overlay_visibility_all();
    update_layout_if_dirty();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
    update_child_objects(delta);
    cleanup_destroyed_children();
    prune_overlays();
    sync_overlay_visibility_all();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
}

void UiWindow::on_ui_input_frame(const UiInputFrame& input)
{
    cleanup_destroyed_children();
    prune_overlays();
    sync_overlay_visibility_all();
    update_layout_if_dirty();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();

    if (OverlayEntry* overlay = active_modal_overlay())
    {
        sync_overlay_visibility(*overlay);
        (void)focus_overlay(*overlay);
        apply_scope_focus();
        if (UiInputFrameReceiver* receiver = dynamic_cast<UiInputFrameReceiver*>(overlay->element))
            receiver->on_ui_input_frame(input);
        cleanup_destroyed_children();
        prune_overlays();
        prune_focus_scopes();
        ensure_valid_scope_focus();
        apply_scope_focus();
        return;
    }

    dispatch_frame_to_children(input);
    cleanup_destroyed_children();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();
}

bool UiWindow::on_ui_input_event(const UiInputEvent& event)
{
    update_focus_input_device(event.device);
    cleanup_destroyed_children();
    prune_overlays();
    sync_overlay_visibility_all();
    update_layout_if_dirty();
    prune_focus_scopes();
    ensure_valid_scope_focus();
    apply_scope_focus();

    if (OverlayEntry* overlay = active_modal_overlay())
    {
        (void)focus_overlay(*overlay);
        apply_scope_focus();

        const bool blocks_background_input = overlay->options.modal;
        if (should_close_overlay_from_event(*overlay,event))
        {
            set_overlay_open(*overlay->element,false);
            return true;
        }

        const bool handled = dispatch_to_overlay(*overlay,event);
        cleanup_destroyed_children();
        prune_overlays();
        prune_focus_scopes();
        ensure_valid_scope_focus();
        apply_scope_focus();
        return handled || blocks_background_input;
    }

    if (OverlayEntry* overlay = active_overlay())
    {
        if (should_close_overlay_from_event(*overlay,event))
        {
            set_overlay_open(*overlay->element,false);
            return true;
        }

        if (dispatch_to_overlay(*overlay,event))
        {
            cleanup_destroyed_children();
            prune_overlays();
            prune_focus_scopes();
            ensure_valid_scope_focus();
            apply_scope_focus();
            return true;
        }
    }

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

        if ((event.type == UiInputEventType::MouseMoved || event.type == UiInputEventType::MouseWheel) && _hover_focus_enabled)
            (void)set_focused_scope_internal(pointer_scope);
        else if (event.type == UiInputEventType::PointerPressed
            && event.device == elysia::input::InputDevice::Mouse
            && event.control == elysia::input::RawInputControl::MouseLeft)
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
    prune_overlays();
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
    self->prune_overlays();
    self->sync_overlay_visibility_all();
    self->update_layout_if_dirty();
    self->prune_focus_scopes();
    self->ensure_valid_scope_focus();
    self->apply_scope_focus();

    const UiWindowStyle& style = _style_state.effective_style();
    if (style.draw_background)
        out_commands.push_back(elysia::core::make_ui_fill_rect_command(screen_rect(),apply_opacity(style.background)));
    submit_child_render_commands(out_commands);
    if (style.draw_border)
        out_commands.push_back(elysia::core::make_ui_draw_rect_command(screen_rect(),apply_opacity(style.border)));
}

void UiWindow::rebuild_layout()
{
    layout::layout_anchored_children(children(),content_rect());
    apply_overlay_placements();
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
    if (_last_focused_scope && !contains_scope(live_scopes,_last_focused_scope))
        _last_focused_scope = nullptr;
}

void UiWindow::prune_overlays()
{
    _overlay_entries.erase(std::remove_if(_overlay_entries.begin(),_overlay_entries.end(),[this](const OverlayEntry& entry)
    {
        return !entry.element || entry.element->is_destroyed() || !is_live_child_element(*entry.element);
    }),_overlay_entries.end());
}

void UiWindow::ensure_valid_scope_focus()
{
    if (_focused_scope && is_registered_scope(*_focused_scope) && is_scope_usable(_focused_scope))
        return;

    _focused_scope = nullptr;
    if (uses_pointer_focus_policy(_focus_input_device))
        return;

    if (restore_preferred_scope_focus())
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
    _last_focused_scope = scope;
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

void UiWindow::update_focus_input_device(elysia::input::InputDevice device) noexcept
{
    if (device != elysia::input::InputDevice::Unknown)
        _focus_input_device = device;
}

bool UiWindow::restore_preferred_scope_focus()
{
    if (_last_focused_scope && is_registered_scope(*_last_focused_scope) && is_scope_usable(_last_focused_scope))
        return set_focused_scope_internal(_last_focused_scope);
    return false;
}

UiWindow::OverlayEntry* UiWindow::active_overlay() noexcept
{
    OverlayEntry* top_overlay = nullptr;
    for (std::size_t index = _overlay_entries.size(); index > 0; --index)
    {
        OverlayEntry& entry = _overlay_entries[index - 1];
        if (!entry.element || !entry.options.open
            || entry.element->is_destroyed())
        {
            continue;
        }

        if (!top_overlay || entry.element->order() > top_overlay->element->order())
            top_overlay = &entry;
    }
    return top_overlay;
}

const UiWindow::OverlayEntry* UiWindow::active_overlay() const noexcept
{
    const OverlayEntry* top_overlay = nullptr;
    for (std::size_t index = _overlay_entries.size(); index > 0; --index)
    {
        const OverlayEntry& entry = _overlay_entries[index - 1];
        if (!entry.element || !entry.options.open
            || entry.element->is_destroyed())
        {
            continue;
        }

        if (!top_overlay || entry.element->order() > top_overlay->element->order())
            top_overlay = &entry;
    }
    return top_overlay;
}

UiWindow::OverlayEntry* UiWindow::active_modal_overlay() noexcept
{
    OverlayEntry* top_overlay = nullptr;
    for (std::size_t index = _overlay_entries.size(); index > 0; --index)
    {
        OverlayEntry& entry = _overlay_entries[index - 1];
        if (!entry.element || !entry.options.open || !entry.options.modal
            || entry.element->is_destroyed())
        {
            continue;
        }

        if (!top_overlay || entry.element->order() > top_overlay->element->order())
            top_overlay = &entry;
    }
    return top_overlay;
}

const UiWindow::OverlayEntry* UiWindow::active_modal_overlay() const noexcept
{
    const OverlayEntry* top_overlay = nullptr;
    for (std::size_t index = _overlay_entries.size(); index > 0; --index)
    {
        const OverlayEntry& entry = _overlay_entries[index - 1];
        if (!entry.element || !entry.options.open || !entry.options.modal
            || entry.element->is_destroyed())
        {
            continue;
        }

        if (!top_overlay || entry.element->order() > top_overlay->element->order())
            top_overlay = &entry;
    }
    return top_overlay;
}

UiWindow::OverlayEntry* UiWindow::find_overlay(UiElement& element) noexcept
{
    auto found = std::find_if(_overlay_entries.begin(),_overlay_entries.end(),[&element](const OverlayEntry& entry)
    {
        return entry.element == &element;
    });
    return found != _overlay_entries.end() ? &(*found) : nullptr;
}

const UiWindow::OverlayEntry* UiWindow::find_overlay(const UiElement& element) const noexcept
{
    auto found = std::find_if(_overlay_entries.begin(),_overlay_entries.end(),[&element](const OverlayEntry& entry)
    {
        return entry.element == &element;
    });
    return found != _overlay_entries.end() ? &(*found) : nullptr;
}

UiFocusScope* UiWindow::overlay_focus_scope(OverlayEntry& entry) noexcept
{
    return entry.element ? dynamic_cast<UiFocusScope*>(entry.element) : nullptr;
}

const UiFocusScope* UiWindow::overlay_focus_scope(const OverlayEntry& entry) const noexcept
{
    return entry.element ? dynamic_cast<const UiFocusScope*>(entry.element) : nullptr;
}

void UiWindow::remember_overlay_restore_focus(OverlayEntry& entry) noexcept
{
    UiFocusScope* overlay_scope = overlay_focus_scope(entry);
    if (_focused_scope && _focused_scope != overlay_scope && is_registered_scope(*_focused_scope) && is_scope_usable(_focused_scope))
        entry.restore_focus_scope = _focused_scope;
}

bool UiWindow::restore_focus_after_overlay_close(OverlayEntry& entry)
{
    UiFocusScope* restore_scope = entry.restore_focus_scope;
    entry.restore_focus_scope = nullptr;
    if (restore_scope && is_registered_scope(*restore_scope) && is_scope_usable(restore_scope))
        return set_focused_scope_internal(restore_scope);
    return false;
}

bool UiWindow::focus_overlay(OverlayEntry& entry)
{
    UiFocusScope* scope = overlay_focus_scope(entry);
    if (!scope)
        return false;

    const bool pointer_focus = uses_pointer_focus_policy(_focus_input_device);
    auto* host = dynamic_cast<UiControlFocusScopeHost*>(scope);

    if (_focused_scope == scope && is_scope_usable(scope))
    {
        if (!pointer_focus && !scope->focused_target())
            return scope->focus_first_available();
        return true;
    }

    register_focus_scope(*scope);
    if (!scope->has_focusable_target())
        return false;

    const bool focused = set_focused_scope_internal(scope);
    if (focused && pointer_focus && host)
        host->set_focused_target(nullptr);
    apply_scope_focus();
    if (focused && !pointer_focus && !scope->focused_target())
        return scope->focus_first_available();
    return focused;
}

bool UiWindow::dispatch_to_overlay(OverlayEntry& entry,const UiInputEvent& event)
{
    if (!entry.element || entry.element->is_destroyed() || !entry.options.open)
        return false;

    sync_overlay_visibility(entry);

    if (UiInputEventReceiver* receiver = dynamic_cast<UiInputEventReceiver*>(entry.element))
        return receiver->on_ui_input_event(event);

    return false;
}

void UiWindow::sync_overlay_visibility(OverlayEntry& entry) noexcept
{
    if (!entry.element)
        return;
    entry.element->set_visible(entry.options.open);
    entry.element->set_active(entry.options.open);
}

void UiWindow::sync_overlay_visibility_all() noexcept
{
    for (OverlayEntry& entry : _overlay_entries)
        sync_overlay_visibility(entry);
}

void UiWindow::apply_overlay_placements() noexcept
{
    for (OverlayEntry& entry : _overlay_entries)
        apply_overlay_placement(entry);
}

void UiWindow::apply_overlay_placement(OverlayEntry& entry) noexcept
{
    if (!entry.element || !entry.options.open)
        return;

    const elysia::core::Rect bounds = content_rect();
    const elysia::core::Vector2 fallback = layout::clamp_size(entry.options.fallback_size);
    const elysia::core::Vector2 current = layout::clamp_size(entry.element->size());
    const float width = current.x > elysia::core::Vector2::k_epsilon ? current.x : fallback.x;
    const float height = current.y > elysia::core::Vector2::k_epsilon ? current.y : fallback.y;

    switch (entry.options.placement)
    {
    case UiOverlayPlacement::LeftDrawer:
        entry.element->set_screen_rect(elysia::core::Rect(bounds.left(),bounds.top(),width,bounds.height()));
        break;
    case UiOverlayPlacement::RightDrawer:
        entry.element->set_screen_rect(elysia::core::Rect(bounds.right() - width,bounds.top(),width,bounds.height()));
        break;
    case UiOverlayPlacement::TopSheet:
        entry.element->set_screen_rect(elysia::core::Rect(bounds.left(),bounds.top(),bounds.width(),height));
        break;
    case UiOverlayPlacement::BottomSheet:
        entry.element->set_screen_rect(elysia::core::Rect(bounds.left(),bounds.bottom() - height,bounds.width(),height));
        break;
    case UiOverlayPlacement::Center:
    default:
        entry.element->set_screen_rect(elysia::core::Rect::from_center(bounds.center(),elysia::core::Vector2(width,height)));
        break;
    }
}

bool UiWindow::should_close_overlay_from_event(const OverlayEntry& entry,const UiInputEvent& event) const noexcept
{
    if (entry.options.close_on_cancel && event.type == UiInputEventType::ActionPressed && event.action == UiAction::Cancel)
        return true;
    if (entry.options.close_on_outside_click && is_primary_mouse_press(event))
        return !contains_overlay_point(entry,event.mouse_x,event.mouse_y);
    return false;
}

bool UiWindow::contains_overlay_point(const OverlayEntry& entry,int mouse_x,int mouse_y) const noexcept
{
    if (!entry.element)
        return false;
    if (entry.element->screen_rect().is_empty())
        return true;
    return entry.element->screen_rect().contains(elysia::core::Vector2(static_cast<float>(mouse_x),static_cast<float>(mouse_y)));
}

bool UiWindow::is_live_child_element(const UiElement& element) const noexcept
{
    return contains_child_element(*this,element);
}

bool UiWindow::uses_pointer_focus_policy(elysia::input::InputDevice device) noexcept
{
    return device == elysia::input::InputDevice::Mouse;
}

void UiWindow::apply_theme(const UiTheme& theme)
{
    _style_state.set_theme_style(theme.window_style);
}
}


