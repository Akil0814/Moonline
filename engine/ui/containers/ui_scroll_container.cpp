#include "ui_scroll_container.h"

#include <algorithm>

namespace elysia::ui
{
namespace
{
[[nodiscard]] float clamp_non_negative(float value) noexcept
{
    return std::max(0.0f,value);
}
}

UiScrollContainer::UiScrollContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiContainer(rect,order) {}

UiScrollContainer::UiScrollContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiContainer(position,size,order) {}

UiScrollContainer::UiScrollContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiContainer(center,size,from_center,order) {}

void UiScrollContainer::reset() noexcept
{
    UiContainer::reset();
    UiContainer::set_clip_children(true);
    _content_height = 0.0f;
    _scroll_offset = 0.0f;
    _scroll_step = 24.0f;
}

bool UiScrollContainer::on_ui_input_event(const UiInputEvent& event)
{
    UiContainer::set_clip_children(true);
    update_layout_if_dirty();

    if (event.type == UiInputEventType::MouseWheel)
    {
        const elysia::core::Rect viewport = content_rect();
        const elysia::core::Vector2 point(static_cast<float>(event.mouse_x),static_cast<float>(event.mouse_y));
        if (viewport.contains(point) && event.wheel_y != 0)
        {
            set_scroll_offset(_scroll_offset - static_cast<float>(event.wheel_y) * _scroll_step);
            return true;
        }
    }

    return UiContainer::on_ui_input_event(event);
}

UiElement* UiScrollContainer::set_content(std::unique_ptr<UiElement> content)
{
    clear_children();
    return add_child(std::move(content));
}

UiElement* UiScrollContainer::content() noexcept
{
    return child_at(0);
}

const UiElement* UiScrollContainer::content() const noexcept
{
    return child_at(0);
}

void UiScrollContainer::clear_content()
{
    clear_children();
}

void UiScrollContainer::set_content_height(float content_height) noexcept
{
    _content_height = clamp_non_negative(content_height);
    _scroll_offset = std::clamp(_scroll_offset,0.0f,max_scroll_offset());
    mark_layout_dirty();
}

float UiScrollContainer::content_height() const noexcept
{
    return _content_height;
}

void UiScrollContainer::set_scroll_offset(float scroll_offset) noexcept
{
    _scroll_offset = std::clamp(scroll_offset,0.0f,max_scroll_offset());
    mark_layout_dirty();
}

float UiScrollContainer::scroll_offset() const noexcept
{
    return _scroll_offset;
}

void UiScrollContainer::set_scroll_step(float scroll_step) noexcept
{
    _scroll_step = clamp_non_negative(scroll_step);
}

float UiScrollContainer::scroll_step() const noexcept
{
    return _scroll_step;
}

void UiScrollContainer::scroll_to_top() noexcept
{
    set_scroll_offset(0.0f);
}

void UiScrollContainer::scroll_to_bottom() noexcept
{
    set_scroll_offset(max_scroll_offset());
}

void UiScrollContainer::rebuild_layout()
{
    UiContainer::set_clip_children(true);
    _scroll_offset = std::clamp(_scroll_offset,0.0f,max_scroll_offset());

    std::vector<ChildEntry>& child_entries = children();
    const elysia::core::Rect viewport = content_rect();
    for (std::size_t index = 0; index < child_entries.size(); ++index)
    {
        ChildEntry& entry = child_entries[index];
        if (!entry.element)
            continue;
        if (index == 0)
        {
            entry.element->set_screen_rect(elysia::core::Rect(
                viewport.x(),
                viewport.y() - _scroll_offset,
                viewport.width(),
                _content_height
            ));
        }
        else
        {
            entry.element->set_screen_rect(elysia::core::Rect::zero());
        }
    }
}

float UiScrollContainer::max_scroll_offset() const noexcept
{
    return std::max(0.0f,_content_height - content_rect().height());
}
}
