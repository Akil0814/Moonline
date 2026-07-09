#include "ui_radio_group.h"

#include <utility>

namespace elysia::ui
{
UiRadioGroup::UiRadioGroup(const elysia::core::Rect& rect,int order) noexcept
    : UiListContainer(rect,order) {}

UiRadioGroup::UiRadioGroup(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiListContainer(position,size,order) {}

UiRadioGroup::UiRadioGroup(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiListContainer(center,size,from_center,order) {}

void UiRadioGroup::reset() noexcept
{
    UiListContainer::reset();
    _selected_index.reset();
    _on_selection_changed = nullptr;
    _is_syncing_selection = false;
}

void UiRadioGroup::update(double delta)
{
    sync_selection(true);
    UiListContainer::update(delta);
    sync_selection(true);
}

void UiRadioGroup::on_ui_input_frame(const UiInputFrame& input)
{
    sync_selection(true);
    UiListContainer::on_ui_input_frame(input);
    sync_selection(true);
}

bool UiRadioGroup::on_ui_input_event(const UiInputEvent& event)
{
    sync_selection(true);
    const bool handled = UiListContainer::on_ui_input_event(event);
    sync_selection(true);
    return handled;
}

void UiRadioGroup::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    auto* self = const_cast<UiRadioGroup*>(this);
    self->cleanup_destroyed_children();
    self->sync_selection(true);
    UiListContainer::submit_ui_render_commands(out_commands);
}

std::optional<std::size_t> UiRadioGroup::selected_index() const noexcept
{
    auto* self = const_cast<UiRadioGroup*>(this);
    self->cleanup_destroyed_children();
    self->sync_selection(false);
    return _selected_index;
}

bool UiRadioGroup::set_selected_index(std::size_t index)
{
    cleanup_destroyed_children();
    UiRadioButton* target = radio_button_at(index);
    if (!target)
        return false;

    for (std::size_t child_index = 0; child_index < child_count(); ++child_index)
    {
        UiRadioButton* button = radio_button_at(child_index);
        if (!button)
            continue;
        button->set_selected(child_index == index);
    }

    sync_selection(true);
    return _selected_index && *_selected_index == index;
}

void UiRadioGroup::set_on_selection_changed(UiRadioGroupSelectionChangedCallback on_selection_changed)
{
    _on_selection_changed = std::move(on_selection_changed);
}

void UiRadioGroup::sync_selection(bool notify)
{
    if (_is_syncing_selection)
        return;

    _is_syncing_selection = true;

    std::optional<std::size_t> first_selected = std::nullopt;
    std::optional<std::size_t> first_radio = std::nullopt;
    std::optional<std::size_t> preferred_selected = std::nullopt;

    if (UiControl* focused = UiListContainer::focused_target())
    {
        for (std::size_t index = 0; index < child_count(); ++index)
        {
            UiRadioButton* button = radio_button_at(index);
            if (button == focused)
            {
                preferred_selected = index;
                break;
            }
        }
    }

    for (std::size_t index = 0; index < child_count(); ++index)
    {
        UiRadioButton* button = radio_button_at(index);
        if (!button)
            continue;

        if (!first_radio)
            first_radio = index;

        if (button->is_selected())
        {
            if (!first_selected)
                first_selected = index;
        }
    }

    std::optional<std::size_t> keep_selected = first_selected;
    if (preferred_selected)
    {
        if (UiRadioButton* preferred_button = radio_button_at(*preferred_selected))
        {
            if (preferred_button->is_selected())
                keep_selected = preferred_selected;
        }
    }

    for (std::size_t index = 0; index < child_count(); ++index)
    {
        UiRadioButton* button = radio_button_at(index);
        if (!button || !button->is_selected())
            continue;
        if (!keep_selected || index != *keep_selected)
            button->set_selected(false);
    }

    if (!keep_selected && first_radio)
    {
        if (UiRadioButton* button = radio_button_at(*first_radio))
        {
            button->set_selected(true);
            keep_selected = first_radio;
        }
    }

    const std::optional<std::size_t> previous = _selected_index;
    _selected_index = keep_selected;

    _is_syncing_selection = false;

    if (notify && previous != _selected_index && _on_selection_changed)
        _on_selection_changed(_selected_index);
}

UiRadioButton* UiRadioGroup::radio_button_at(std::size_t index) const noexcept
{
    const UiElement* child = child_at(index);
    return child ? dynamic_cast<UiRadioButton*>(const_cast<UiElement*>(child)) : nullptr;
}
}
