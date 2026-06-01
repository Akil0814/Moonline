#include "ui_multi_select_button_group.h"

#include <algorithm>

UiMultiSelectButtonGroup::~UiMultiSelectButtonGroup()
{
    clear_buttons();
}

void UiMultiSelectButtonGroup::add_button(const std::shared_ptr<UiSelectableButton>& button)
{
    if (!button)
    {
        return;
    }

    cleanup_buttons();
    if (button_index(button.get()) >= 0)
    {
        return;
    }

    ButtonRegistration registration;
    registration._button = button;
    button->set_activation_behavior(UiSelectableActivationBehavior::Toggle);
    registration._listener_id = button->add_selection_listener(
        [this](UiSelectableButton& selectable_button, bool selected)
        {
            handle_button_selection_changed(&selectable_button, selected);
        }
    );
    _buttons.push_back(std::move(registration));
}

bool UiMultiSelectButtonGroup::remove_button(const UiSelectableButton* button)
{
    cleanup_buttons();
    const int index = button_index(button);
    if (index < 0)
    {
        return false;
    }

    const std::shared_ptr<UiSelectableButton> button_handle = _buttons[static_cast<size_t>(index)]._button.lock();
    if (button_handle)
    {
        button_handle->remove_selection_listener(_buttons[static_cast<size_t>(index)]._listener_id);
    }

    _buttons.erase(_buttons.begin() + index);
    return true;
}

void UiMultiSelectButtonGroup::clear_buttons()
{
    for (ButtonRegistration& registration : _buttons)
    {
        const std::shared_ptr<UiSelectableButton> button = registration._button.lock();
        if (button)
        {
            button->remove_selection_listener(registration._listener_id);
        }
    }

    _buttons.clear();
}

void UiMultiSelectButtonGroup::clear_selection()
{
    _is_syncing_selection = true;
    for (ButtonRegistration& registration : _buttons)
    {
        const std::shared_ptr<UiSelectableButton> button = registration._button.lock();
        if (button)
        {
            button->set_selected(false);
        }
    }
    _is_syncing_selection = false;
}

size_t UiMultiSelectButtonGroup::button_count() const
{
    return _buttons.size();
}

bool UiMultiSelectButtonGroup::is_button_selected(int index) const
{
    const std::shared_ptr<UiSelectableButton> button = button_at(index);
    return button ? button->is_selected() : false;
}

bool UiMultiSelectButtonGroup::set_button_selected(int index, bool selected)
{
    const std::shared_ptr<UiSelectableButton> button = button_at(index);
    if (!button)
    {
        return false;
    }

    button->set_selected(selected);
    return true;
}

bool UiMultiSelectButtonGroup::toggle_button_selected(int index)
{
    const std::shared_ptr<UiSelectableButton> button = button_at(index);
    if (!button)
    {
        return false;
    }

    button->set_selected(!button->is_selected());
    return true;
}

std::vector<int> UiMultiSelectButtonGroup::selected_indices() const
{
    std::vector<int> indices;
    for (int index = 0; index < static_cast<int>(_buttons.size()); ++index)
    {
        const std::shared_ptr<UiSelectableButton> button = _buttons[static_cast<size_t>(index)]._button.lock();
        if (button && button->is_selected())
        {
            indices.push_back(index);
        }
    }

    return indices;
}

std::vector<std::shared_ptr<UiSelectableButton>> UiMultiSelectButtonGroup::selected_buttons() const
{
    std::vector<std::shared_ptr<UiSelectableButton>> buttons;
    for (const ButtonRegistration& registration : _buttons)
    {
        const std::shared_ptr<UiSelectableButton> button = registration._button.lock();
        if (button && button->is_selected())
        {
            buttons.push_back(button);
        }
    }

    return buttons;
}

void UiMultiSelectButtonGroup::set_on_selection_changed(
    UiMultiSelectButtonGroupSelectionChangedCallback on_selection_changed
)
{
    _on_selection_changed = std::move(on_selection_changed);
}

void UiMultiSelectButtonGroup::cleanup_buttons()
{
    _buttons.erase(
        std::remove_if(
            _buttons.begin(),
            _buttons.end(),
            [](const ButtonRegistration& registration)
            {
                return registration._button.expired();
            }
        ),
        _buttons.end()
    );
}

void UiMultiSelectButtonGroup::handle_button_selection_changed(UiSelectableButton* button, bool selected)
{
    if (_is_syncing_selection || !button)
    {
        return;
    }

    const int index = button_index(button);
    if (index < 0)
    {
        return;
    }

    if (_on_selection_changed)
    {
        _on_selection_changed(index, button, selected);
    }
}

int UiMultiSelectButtonGroup::button_index(const UiSelectableButton* button) const
{
    if (!button)
    {
        return -1;
    }

    for (int index = 0; index < static_cast<int>(_buttons.size()); ++index)
    {
        const std::shared_ptr<UiSelectableButton> current_button = _buttons[static_cast<size_t>(index)]._button.lock();
        if (current_button.get() == button)
        {
            return index;
        }
    }

    return -1;
}

std::shared_ptr<UiSelectableButton> UiMultiSelectButtonGroup::button_at(int index) const
{
    if (index < 0 || index >= static_cast<int>(_buttons.size()))
    {
        return nullptr;
    }

    return _buttons[static_cast<size_t>(index)]._button.lock();
}
