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
    _selected_button = nullptr;
    _on_selection_changed = nullptr;
    _is_syncing_selection = false;
    _selection_notification_pending = false;
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
    self->sync_selection(false);
    UiListContainer::submit_ui_render_commands(out_commands);
}

std::optional<std::size_t> UiRadioGroup::selected_index() const noexcept
{
    return find_radio_index(_selected_button);
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
    return _selected_button == target;
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

    UiRadioButton* first_selected = nullptr;
    UiRadioButton* first_radio = nullptr;
    UiRadioButton* preferred_selected = nullptr;

    if (UiControl* focused = UiListContainer::focused_target())
    {
        for (std::size_t index = 0; index < child_count(); ++index)
        {
            UiRadioButton* button = radio_button_at(index);
            if (button == focused)
            {
                preferred_selected = button;
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
            first_radio = button;

        if (button->is_selected())
        {
            if (!first_selected)
                first_selected = button;
        }
    }

    UiRadioButton* keep_selected = nullptr;
    if (_selected_button && find_radio_index(_selected_button) && _selected_button->is_selected())
        keep_selected = _selected_button;
    if (preferred_selected && preferred_selected->is_selected())
        keep_selected = preferred_selected;
    if (!keep_selected)
        keep_selected = first_selected;

    for (std::size_t index = 0; index < child_count(); ++index)
    {
        UiRadioButton* button = radio_button_at(index);
        if (!button || !button->is_selected())
            continue;
        if (button != keep_selected)
            button->set_selected(false);
    }

    if (!keep_selected && first_radio)
    {
        first_radio->set_selected(true);
        keep_selected = first_radio;
    }

    UiRadioButton* previous = _selected_button;
    _selected_button = keep_selected;

    _is_syncing_selection = false;

    const bool changed = previous != _selected_button;
    if (changed && !notify)
        _selection_notification_pending = true;
    if (notify && (changed || _selection_notification_pending))
    {
        _selection_notification_pending = false;
        if (_on_selection_changed)
            _on_selection_changed(selected_index());
    }
}

std::optional<std::size_t> UiRadioGroup::find_radio_index(const UiRadioButton* button) const noexcept
{
    if (!button)
        return std::nullopt;
    for (std::size_t index = 0; index < child_count(); ++index)
    {
        if (radio_button_at(index) == button)
            return index;
    }
    return std::nullopt;
}

UiRadioButton* UiRadioGroup::radio_button_at(std::size_t index) const noexcept
{
    const UiElement* child = child_at(index);
    return child ? dynamic_cast<UiRadioButton*>(const_cast<UiElement*>(child)) : nullptr;
}
}
