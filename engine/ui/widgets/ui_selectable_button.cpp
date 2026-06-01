#include "ui_selectable_button.h"

#include <algorithm>
#include <utility>

UiSelectableButton::UiSelectableButton(Vector2 position, Vector2 size, int order)
    : UiTextButton(position, size, order)
{
}

void UiSelectableButton::reset()
{
    UiTextButton::reset();
    _activation_behavior = UiSelectableActivationBehavior::Select;
    _selected = false;
}

void UiSelectableButton::set_selected(bool selected)
{
    set_selected_internal(selected, true);
}

bool UiSelectableButton::is_selected() const
{
    return _selected;
}

UiSelectableButtonListenerId UiSelectableButton::add_selection_listener(
    UiSelectableButtonSelectionChangedCallback callback
)
{
    if (!callback)
    {
        return 0;
    }

    SelectionListenerEntry entry;
    entry._id = _next_listener_id++;
    entry._callback = std::move(callback);
    _selection_listeners.push_back(std::move(entry));
    return _selection_listeners.back()._id;
}

bool UiSelectableButton::remove_selection_listener(UiSelectableButtonListenerId listener_id)
{
    const size_t original_size = _selection_listeners.size();
    _selection_listeners.erase(
        std::remove_if(
            _selection_listeners.begin(),
            _selection_listeners.end(),
            [listener_id](const SelectionListenerEntry& entry)
            {
                return entry._id == listener_id;
            }
        ),
        _selection_listeners.end()
    );
    return _selection_listeners.size() != original_size;
}

void UiSelectableButton::clear_selection_listeners()
{
    _selection_listeners.clear();
}

void UiSelectableButton::set_activation_behavior(UiSelectableActivationBehavior activation_behavior)
{
    _activation_behavior = activation_behavior;
}

UiSelectableActivationBehavior UiSelectableButton::activation_behavior() const
{
    return _activation_behavior;
}

void UiSelectableButton::on_activated()
{
    if (_activation_behavior == UiSelectableActivationBehavior::Toggle)
    {
        set_selected_internal(!_selected, true);
        return;
    }

    set_selected_internal(true, true);
}

UiButton::Status UiSelectableButton::visual_status() const
{
    if (_selected && status() != Status::Pushed)
    {
        return Status::Pushed;
    }

    return UiButton::visual_status();
}

void UiSelectableButton::set_selected_internal(bool selected, bool notify)
{
    if (_selected == selected)
    {
        return;
    }

    _selected = selected;
    if (!notify)
    {
        return;
    }

    for (SelectionListenerEntry& entry : _selection_listeners)
    {
        if (entry._callback)
        {
            entry._callback(*this, _selected);
        }
    }
}
