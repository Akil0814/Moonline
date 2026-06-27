#include "ui_container.h"

#include <algorithm>

namespace elysia::ui
{
UiContainer::UiContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiElement(rect,order) {}

UiContainer::UiContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiElement(position,size,order) {}

UiContainer::UiContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiElement(center,size,from_center,order) {}

void UiContainer::reset() noexcept
{
    UiElement::reset();
    _children.clear();
    _padding = UiLayoutPadding{};
    _clip_children = false;
    _layout_dirty = true;
    _last_layout_rect = elysia::core::Rect::zero();
}

UiElement* UiContainer::add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options)
{
    if (!child)
        return nullptr;

    UiElement* child_ptr = child.get();
    _children.push_back(ChildEntry{ std::move(child),options });
    mark_layout_dirty();
    return child_ptr;
}

void UiContainer::clear_children()
{
    _children.clear();
    mark_layout_dirty();
}

std::size_t UiContainer::child_count() const noexcept
{
    return _children.size();
}

UiElement* UiContainer::child_at(std::size_t index) noexcept
{
    return index < _children.size() ? _children[index].element.get() : nullptr;
}

const UiElement* UiContainer::child_at(std::size_t index) const noexcept
{
    return index < _children.size() ? _children[index].element.get() : nullptr;
}

void UiContainer::set_child_layout_options(std::size_t index,const UiLayoutChildOptions& options)
{
    if (index >= _children.size())
        return;
    _children[index].layout = options;
    mark_layout_dirty();
}

const UiLayoutChildOptions* UiContainer::child_layout_options(std::size_t index) const noexcept
{
    return index < _children.size() ? &_children[index].layout : nullptr;
}

void UiContainer::set_padding(const UiLayoutPadding& padding) noexcept
{
    _padding = padding;
    mark_layout_dirty();
}

const UiLayoutPadding& UiContainer::padding() const noexcept
{
    return _padding;
}

void UiContainer::set_clip_children(bool clip_children) noexcept
{
    _clip_children = clip_children;
}

bool UiContainer::clips_children() const noexcept
{
    return _clip_children;
}

void UiContainer::mark_layout_dirty() noexcept
{
    _layout_dirty = true;
}

void UiContainer::update_layout_if_dirty()
{
    if (!needs_layout_rebuild())
        return;

    rebuild_layout();
    _last_layout_rect = screen_rect();
    _layout_dirty = false;
}

void UiContainer::update(double delta)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();

    for (ChildEntry& entry : _children)
    {
        if (!entry.element || entry.element->is_destroyed() || !entry.element->is_active())
            continue;
        if (elysia::core::Updatable* updatable = dynamic_cast<elysia::core::Updatable*>(entry.element.get()))
            updatable->update(delta);
    }
}

void UiContainer::on_ui_input_frame(const UiInputFrame& input)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();

    for (ChildEntry& entry : _children)
    {
        if (!entry.element || entry.element->is_destroyed() || !entry.element->is_active())
            continue;
        if (UiInputFrameReceiver* receiver = dynamic_cast<UiInputFrameReceiver*>(entry.element.get()))
            receiver->on_ui_input_frame(input);
    }
}

bool UiContainer::on_ui_input_event(const UiInputEvent& event)
{
    cleanup_destroyed_children();
    update_layout_if_dirty();

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

void UiContainer::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    const_cast<UiContainer*>(this)->update_layout_if_dirty();
    submit_child_render_commands(out_commands);
}

void UiContainer::rebuild_layout()
{
}

elysia::core::Rect UiContainer::content_rect() const noexcept
{
    const elysia::core::Rect& rect = screen_rect();
    const float width = std::max(0.0f,rect.width() - _padding.left - _padding.right);
    const float height = std::max(0.0f,rect.height() - _padding.top - _padding.bottom);
    return elysia::core::Rect(rect.x() + _padding.left,rect.y() + _padding.top,width,height);
}

std::vector<UiContainer::ChildEntry>& UiContainer::children() noexcept
{
    return _children;
}

const std::vector<UiContainer::ChildEntry>& UiContainer::children() const noexcept
{
    return _children;
}

void UiContainer::submit_child_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
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

void UiContainer::apply_opacity_to_range(std::vector<elysia::core::UiRenderCommand>& out_commands,std::size_t begin) const
{
    if (opacity() == 255)
        return;

    for (std::size_t index = begin; index < out_commands.size(); ++index)
    {
        elysia::core::UiRenderCommand& command = out_commands[index];
        switch (command.type)
        {
        case elysia::core::UiRenderCommandType::Texture:
            command.alpha = multiply_alpha(command.alpha,opacity());
            break;
        case elysia::core::UiRenderCommandType::FillRect:
        case elysia::core::UiRenderCommandType::DrawRect:
        case elysia::core::UiRenderCommandType::DrawLine:
            command.color.a = multiply_alpha(command.color.a,opacity());
            break;
        default:
            break;
        }
    }
}

void UiContainer::apply_clip_to_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    const elysia::core::Rect& clip_rect
) const
{
    if (clip_rect.is_empty())
    {
        out_commands.erase(out_commands.begin() + static_cast<std::ptrdiff_t>(begin),out_commands.end());
        return;
    }

    for (std::size_t index = out_commands.size(); index > begin; --index)
    {
        elysia::core::UiRenderCommand& command = out_commands[index - 1];
        const elysia::core::Rect final_clip = command.use_clip_rect ? command.clip_rect.intersection(clip_rect) : clip_rect;
        if (final_clip.is_empty())
        {
            out_commands.erase(out_commands.begin() + static_cast<std::ptrdiff_t>(index - 1));
            continue;
        }

        command.use_clip_rect = true;
        command.clip_rect = final_clip;
    }
}

void UiContainer::finalize_child_command_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    const elysia::core::Rect& clip_rect
) const
{
    if (begin >= out_commands.size())
        return;

    apply_opacity_to_range(out_commands,begin);
    if (clips_children())
        apply_clip_to_range(out_commands,begin,clip_rect);
}

void UiContainer::cleanup_destroyed_children()
{
    const std::size_t previous_count = _children.size();
    std::erase_if(_children,[](const ChildEntry& entry)
    {
        return !entry.element || entry.element->is_destroyed();
    });
    if (_children.size() != previous_count)
        mark_layout_dirty();
}

bool UiContainer::needs_layout_rebuild() const noexcept
{
    return _layout_dirty || !_last_layout_rect.nearly_equals(screen_rect());
}

std::uint8_t UiContainer::multiply_alpha(std::uint8_t a,std::uint8_t b) noexcept
{
    return static_cast<std::uint8_t>((static_cast<unsigned int>(a) * static_cast<unsigned int>(b)) / 255U);
}
}
