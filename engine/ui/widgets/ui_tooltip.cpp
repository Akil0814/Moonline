#include "ui_tooltip.h"

#include "../core/ui_focusable.h"
#include "../window/ui_window.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
constexpr float tooltip_gap = 8.0f;
}

UiTooltip::UiTooltip(int order) noexcept : UiElement(elysia::core::Rect::zero(),order)
{
    reset();
}

UiTooltip::~UiTooltip()
{
    unregister_from_window();
}

void UiTooltip::reset() noexcept
{
    unregister_from_window();
    UiElement::reset();
    _trigger = nullptr;
    _content.reset();
    _show_delay = 0.4;
    _hover_time = 0.0;
    _mouse_x = 0;
    _mouse_y = 0;
    _has_pointer = false;
    _open = false;
}

void UiTooltip::update(double delta)
{
    if (!_trigger || !_content || !_window || _trigger->is_destroyed()
        || !_trigger->is_visible() || !_trigger->is_active())
    {
        _hover_time = 0.0;
        hide();
        return;
    }

    if (!trigger_is_active())
    {
        _hover_time = 0.0;
        hide();
        return;
    }

    _hover_time += std::max(0.0,delta);
    if (_hover_time >= _show_delay)
        show();

    if (_open)
    {
        sync_content_position();
        if (auto* updatable = dynamic_cast<elysia::core::Updatable*>(_content.get()); updatable && _content->is_active())
            updatable->update(delta);
    }
}

void UiTooltip::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    (void)out_commands;
}

void UiTooltip::bind_trigger(UiElement& trigger) noexcept
{
    if (_trigger == &trigger)
        return;
    _trigger = &trigger;
    _hover_time = 0.0;
    hide();
}

void UiTooltip::clear_trigger() noexcept
{
    _trigger = nullptr;
    _hover_time = 0.0;
    hide();
}

UiElement* UiTooltip::set_content(std::unique_ptr<UiElement> content)
{
    if (!content)
    {
        clear_content();
        return nullptr;
    }
    hide();
    _content = std::move(content);
    return _content.get();
}

std::unique_ptr<UiElement> UiTooltip::release_content() noexcept
{
    hide();
    return std::move(_content);
}

void UiTooltip::clear_content() noexcept
{
    hide();
    _content.reset();
}

void UiTooltip::set_show_delay(double seconds) noexcept
{
    _show_delay = std::max(0.0,seconds);
}

void UiTooltip::show() noexcept
{
    if (!_trigger || !_content)
        return;
    _open = true;
    sync_content_position();
}

void UiTooltip::hide() noexcept
{
    _open = false;
}

void UiTooltip::register_with_window(UiWindow& window)
{
    if (_window == &window)
        return;
    unregister_from_window();
    _window = &window;
    _window->register_tooltip(*this);
}

void UiTooltip::unregister_from_window() noexcept
{
    UiWindow* window = _window;
    _window = nullptr;
    if (window)
        window->unregister_tooltip(*this);
    hide();
}

void UiTooltip::observe_pointer(int mouse_x,int mouse_y) noexcept
{
    _mouse_x = mouse_x;
    _mouse_y = mouse_y;
    _has_pointer = true;
}

void UiTooltip::submit_tooltip_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!_open || !_content || !_content->is_visible() || _content->is_destroyed())
        return;
    const_cast<UiTooltip*>(this)->sync_content_position();
    _content->submit_ui_render_commands(out_commands);
}

void UiTooltip::sync_content_position() noexcept
{
    if (!_trigger || !_content || !_window)
        return;

    const auto& anchor = _trigger->screen_rect();
    const auto& bounds = _window->content_bounds();
    const auto size = _content->size();
    float left = anchor.right() + tooltip_gap;
    float top = anchor.bottom() + tooltip_gap;
    if (left + size.x > bounds.right())
        left = anchor.left() - tooltip_gap - size.x;
    if (top + size.y > bounds.bottom())
        top = anchor.top() - tooltip_gap - size.y;
    left = std::clamp(left,bounds.left(),std::max(bounds.left(),bounds.right() - size.x));
    top = std::clamp(top,bounds.top(),std::max(bounds.top(),bounds.bottom() - size.y));
    _content->set_position(elysia::core::Vector2(left,top));
}

bool UiTooltip::trigger_is_active() const noexcept
{
    const bool pointer_blocked = _window && _has_pointer
        && _window->is_tooltip_pointer_blocked(_mouse_x,_mouse_y);
    const bool hovered = _has_pointer && !pointer_blocked && _trigger->screen_rect().contains(
        elysia::core::Vector2(static_cast<float>(_mouse_x),static_cast<float>(_mouse_y)));
    const auto* focusable = dynamic_cast<const UiFocusable*>(_trigger);
    const bool focus_blocked = _window && _window->blocks_background_tooltips();
    return hovered || (!focus_blocked && focusable && focusable->is_focused());
}
}
