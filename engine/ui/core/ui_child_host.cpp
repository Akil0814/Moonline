#include "ui_child_host.h"
#include "ui_render_command_range_utils.h"
#include "../layout/ui_layout_geometry.h"

#include <algorithm>
#include <cstddef>

namespace elysia::ui
{
UiChildHost::UiChildHost(const elysia::core::Rect& rect,int order) noexcept
    : UiElement(rect,order) {}

UiChildHost::UiChildHost(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiElement(position,size,order) {}

UiChildHost::UiChildHost(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiElement(center,size,from_center,order) {}

void UiChildHost::reset() noexcept
{
    UiElement::reset();
    _children.clear();
    _padding = UiLayoutPadding{};
    _clip_children = false;
    _layout_dirty = true;
    _last_layout_rect = elysia::core::Rect::zero();
}

UiElement* UiChildHost::add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options)
{
    return insert_child(std::move(child),_children.size(),options);
}

UiElement* UiChildHost::insert_child(std::unique_ptr<UiElement> child,std::size_t index,UiLayoutChildOptions options)
{
    if (!child)
        return nullptr;

    UiElement* child_ptr = child.get();
    const std::size_t target_index = std::min(index,_children.size());
    _children.insert(_children.begin() + static_cast<std::ptrdiff_t>(target_index),ChildEntry{ std::move(child),options });
    mark_layout_dirty();
    return child_ptr;
}

void UiChildHost::clear_children()
{
    _children.clear();
    mark_layout_dirty();
}

std::size_t UiChildHost::child_count() const noexcept
{
    return _children.size();
}

UiElement* UiChildHost::child_at(std::size_t index) noexcept
{
    return index < _children.size() ? _children[index].element.get() : nullptr;
}

const UiElement* UiChildHost::child_at(std::size_t index) const noexcept
{
    return index < _children.size() ? _children[index].element.get() : nullptr;
}

void UiChildHost::set_child_layout_options(std::size_t index,const UiLayoutChildOptions& options)
{
    if (index >= _children.size())
        return;
    _children[index].layout = options;
    mark_layout_dirty();
}

const UiLayoutChildOptions* UiChildHost::child_layout_options(std::size_t index) const noexcept
{
    return index < _children.size() ? &_children[index].layout : nullptr;
}

void UiChildHost::set_padding(const UiLayoutPadding& padding) noexcept
{
    _padding = padding;
    mark_layout_dirty();
}

const UiLayoutPadding& UiChildHost::padding() const noexcept
{
    return _padding;
}

void UiChildHost::set_clip_children(bool clip_children) noexcept
{
    _clip_children = clip_children;
}

bool UiChildHost::clips_children() const noexcept
{
    return _clip_children;
}

void UiChildHost::mark_layout_dirty() noexcept
{
    _layout_dirty = true;
}

void UiChildHost::update_layout_if_dirty()
{
    if (!needs_layout_rebuild())
        return;

    rebuild_layout();
    _last_layout_rect = screen_rect();
    _layout_dirty = false;
}

void UiChildHost::update(double delta)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    update_child_objects(delta);
}

void UiChildHost::on_ui_input_frame(const UiInputFrame& input)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    dispatch_frame_to_children(input);
}

bool UiChildHost::on_ui_input_event(const UiInputEvent& event)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();
    return dispatch_input_to_children(event);
}

void UiChildHost::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const_cast<UiChildHost*>(this)->update_layout_if_dirty();
    submit_child_render_commands(out_commands);
}

void UiChildHost::rebuild_layout()
{
}

elysia::core::Rect UiChildHost::content_rect() const noexcept
{
    return layout::padded_content_rect(screen_rect(),_padding);
}

std::vector<UiChildHost::ChildEntry>& UiChildHost::children() noexcept
{
    return _children;
}

const std::vector<UiChildHost::ChildEntry>& UiChildHost::children() const noexcept
{
    return _children;
}

void UiChildHost::submit_child_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    const elysia::core::Rect clip_rect = clips_children() ? content_rect() : elysia::core::Rect::zero();
    for (const ChildEntry& entry : _children)
    {
        if (!entry.element || entry.element->is_destroyed() || !entry.element->is_visible())
            continue;

        const std::size_t begin = out_commands.size();
        entry.element->submit_ui_render_commands(out_commands);
        finalize_child_command_range(out_commands,begin,clip_rect);
    }
}

void UiChildHost::apply_opacity_to_range(std::vector<elysia::core::UiRenderCommand>& out_commands,std::size_t begin) const
{
    render_command_range_utils::apply_opacity_to_range(out_commands,begin,opacity());
}

void UiChildHost::apply_clip_to_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    const elysia::core::Rect& clip_rect
) const
{
    render_command_range_utils::apply_clip_to_range(out_commands,begin,clip_rect);
}

void UiChildHost::finalize_child_command_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    const elysia::core::Rect& clip_rect
) const
{
    render_command_range_utils::finalize_child_command_range(out_commands,begin,opacity(),clips_children(),clip_rect);
}

void UiChildHost::update_child_objects(double delta)
{
    for (ChildEntry& entry : _children)
    {
        if (!entry.element || entry.element->is_destroyed() || !entry.element->is_active())
            continue;
        if (elysia::core::Updatable* updatable = dynamic_cast<elysia::core::Updatable*>(entry.element.get()))
            updatable->update(delta);
    }
}

void UiChildHost::dispatch_frame_to_children(const UiInputFrame& input)
{
    for (ChildEntry& entry : _children)
    {
        if (!entry.element || entry.element->is_destroyed() || !entry.element->is_active())
            continue;
        if (UiInputFrameReceiver* receiver = dynamic_cast<UiInputFrameReceiver*>(entry.element.get()))
            receiver->on_ui_input_frame(input);
    }
}

bool UiChildHost::dispatch_input_to_children(const UiInputEvent& event)
{
    for (std::size_t index = _children.size(); index > 0; --index)
    {
        ChildEntry& entry = _children[index - 1];
        if (!entry.element || entry.element->is_destroyed() || !entry.element->is_active())
            continue;
        if (UiInputEventReceiver* receiver = dynamic_cast<UiInputEventReceiver*>(entry.element.get()))
        {
            if (receiver->on_ui_input_event(event))
                return true;
        }
    }

    return false;
}

void UiChildHost::cleanup_destroyed_children()
{
    const std::size_t previous_count = _children.size();
    std::erase_if(_children,[](const ChildEntry& entry)
    {
        return !entry.element || entry.element->is_destroyed();
    });
    if (_children.size() != previous_count)
        mark_layout_dirty();
}

bool UiChildHost::needs_layout_rebuild() const noexcept
{
    return _layout_dirty || !_last_layout_rect.nearly_equals(screen_rect());
}
}


