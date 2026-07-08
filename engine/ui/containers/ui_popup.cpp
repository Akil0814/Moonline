#include "ui_popup.h"

#include "../focus/ui_focus_scope_utils.h"
#include "../layout/ui_anchor_layout.h"
#include "../style/ui_style_defaults.h"
#include "../../core/render/render_command.h"

namespace elysia::ui
{
UiPopup::UiPopup() noexcept
    : UiControlFocusScopeHost(default_rect(),default_order)
{
    _use_default_centering = true;
    reset();
}

UiPopup::UiPopup(const elysia::core::Rect& rect,int order) noexcept
    : UiControlFocusScopeHost(rect,order)
{
    reset();
}

UiPopup::UiPopup(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiControlFocusScopeHost(position,size,order)
{
    reset();
}

UiPopup::UiPopup(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiControlFocusScopeHost(center,size,from_center,order)
{
    reset();
}

void UiPopup::reset() noexcept
{
    UiControlFocusScopeHost::reset();
    _style = UiStyleDefaults::popup();
    _modal = true;
    _close_on_cancel = true;
    _close_on_outside_click = true;
}

bool UiPopup::on_ui_input_event(const UiInputEvent& event)
{
    if (should_close_from_event(event))
    {
        close();
        return true;
    }

    return UiControlFocusScopeHost::on_ui_input_event(event);
}

void UiPopup::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    auto* self = const_cast<UiPopup*>(this);
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

void UiPopup::set_modal(bool modal) noexcept
{
    _modal = modal;
}

bool UiPopup::is_modal() const noexcept
{
    return _modal;
}

void UiPopup::set_close_on_cancel(bool close_on_cancel) noexcept
{
    _close_on_cancel = close_on_cancel;
}

bool UiPopup::closes_on_cancel() const noexcept
{
    return _close_on_cancel;
}

void UiPopup::set_close_on_outside_click(bool close_on_outside_click) noexcept
{
    _close_on_outside_click = close_on_outside_click;
}

bool UiPopup::closes_on_outside_click() const noexcept
{
    return _close_on_outside_click;
}

void UiPopup::close() noexcept
{
    destroy();
}

bool UiPopup::is_open() const noexcept
{
    return !is_destroyed() && is_active() && is_visible();
}

void UiPopup::set_style(const UiPopupStyle& style) noexcept
{
    _style = style;
}

const UiPopupStyle& UiPopup::style() const noexcept
{
    return _style;
}

void UiPopup::set_draw_background(bool draw_background) noexcept
{
    _style.draw_background = draw_background;
}

bool UiPopup::draws_background() const noexcept
{
    return _style.draw_background;
}

void UiPopup::set_draw_border(bool draw_border) noexcept
{
    _style.draw_border = draw_border;
}

bool UiPopup::draws_border() const noexcept
{
    return _style.draw_border;
}

void UiPopup::set_background_color(elysia::core::Color color) noexcept
{
    _style.background = color;
}

elysia::core::Color UiPopup::background_color() const noexcept
{
    return _style.background;
}

void UiPopup::set_border_color(elysia::core::Color color) noexcept
{
    _style.border = color;
}

elysia::core::Color UiPopup::border_color() const noexcept
{
    return _style.border;
}

bool UiPopup::contains_pointer(int mouse_x,int mouse_y) const noexcept
{
    return screen_rect().contains(elysia::core::Vector2(static_cast<float>(mouse_x),static_cast<float>(mouse_y)));
}

bool UiPopup::should_close_from_event(const UiInputEvent& event) const noexcept
{
    if (_close_on_cancel && event.type == UiInputEventType::ActionPressed && event.action == UiAction::Cancel)
        return true;

    if (_close_on_outside_click
        && event.type == UiInputEventType::PointerPressed
        && event.device == elysia::input::InputDevice::Mouse
        && event.control == elysia::input::RawInputControl::MouseLeft)
    {
        return !contains_pointer(event.mouse_x,event.mouse_y);
    }

    return false;
}

bool UiPopup::uses_default_centering() const noexcept
{
    return _use_default_centering;
}

void UiPopup::rebuild_layout()
{
    layout::layout_anchored_children(children(),content_rect());
}

void UiPopup::rebuild_focus_registry()
{
    std::vector<const UiControl*> live_controls;
    collect_live_controls(*this,live_controls);

    std::vector<FocusEntry> entries;
    entries.reserve(live_controls.size());
    for (std::size_t index = 0; index < live_controls.size(); ++index)
    {
        UiControl* control = const_cast<UiControl*>(live_controls[index]);
        if (!control)
            continue;

        UiFocusNeighbors neighbors;
        if (index > 0)
        {
            UiControl* previous = const_cast<UiControl*>(live_controls[index - 1]);
            neighbors.up = previous;
            neighbors.left = previous;
        }
        if (index + 1 < live_controls.size())
        {
            UiControl* next = const_cast<UiControl*>(live_controls[index + 1]);
            neighbors.down = next;
            neighbors.right = next;
        }
        entries.push_back(FocusEntry{ control,neighbors });
    }

    set_focus_entries(std::move(entries));
}
}
