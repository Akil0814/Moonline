#include "ui_button_group.h"

#include <algorithm>

UiButtonGroup::~UiButtonGroup()
{
    clear_buttons();
}

void UiButtonGroup::add_button(const std::shared_ptr<UiSelectableButton>& button)
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
    button->set_activation_behavior(UiSelectableActivationBehavior::Select);
    registration._listener_id = button->add_selection_listener(
        [this](UiSelectableButton& selectable_button, bool selected)
        {
            handle_button_selection_changed(&selectable_button, selected);
        }
    );
    _buttons.push_back(std::move(registration));

    if (button->is_selected())
    {
        set_selected_index(static_cast<int>(_buttons.size()) - 1);
    }
}

bool UiButtonGroup::remove_button(const UiSelectableButton* button)
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
    if (_selected_index == index)
    {
        _selected_index = -1;
    }
    else if (_selected_index > index)
    {
        --_selected_index;
    }

    if (_on_selection_changed)
    {
        _on_selection_changed(_selected_index, selected_button().get());
    }

    return true;
}

void UiButtonGroup::clear_buttons()
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
    _selected_index = -1;
}

size_t UiButtonGroup::button_count() const
{
    return _buttons.size();
}

void UiButtonGroup::set_selected_index(int index)
{
    cleanup_buttons();

    if (_buttons.empty())
    {
        _selected_index = -1;
        return;
    }

    _is_syncing_selection = true;

    if (index < 0 || index >= static_cast<int>(_buttons.size()))
    {
        for (ButtonRegistration& registration : _buttons)
        {
            const std::shared_ptr<UiSelectableButton> button = registration._button.lock();
            if (button)
            {
                button->set_selected(false);
            }
        }

        _selected_index = -1;
    }
    else
    {
        for (int button_index_value = 0; button_index_value < static_cast<int>(_buttons.size()); ++button_index_value)
        {
            const std::shared_ptr<UiSelectableButton> button = _buttons[static_cast<size_t>(button_index_value)]._button.lock();
            if (!button)
            {
                continue;
            }

            button->set_selected(button_index_value == index);
        }

        _selected_index = index;
    }

    _is_syncing_selection = false;

    if (_on_selection_changed)
    {
        _on_selection_changed(_selected_index, selected_button().get());
    }
}

int UiButtonGroup::selected_index() const
{
    return _selected_index;
}

std::shared_ptr<UiSelectableButton> UiButtonGroup::selected_button() const
{
    if (_selected_index < 0 || _selected_index >= static_cast<int>(_buttons.size()))
    {
        return nullptr;
    }

    return _buttons[static_cast<size_t>(_selected_index)]._button.lock();
}

void UiButtonGroup::set_on_selection_changed(UiButtonGroupSelectionChangedCallback on_selection_changed)
{
    _on_selection_changed = std::move(on_selection_changed);
}

void UiButtonGroup::cleanup_buttons()
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

    if (_selected_index >= static_cast<int>(_buttons.size()))
    {
        _selected_index = static_cast<int>(_buttons.size()) - 1;
    }
}

void UiButtonGroup::handle_button_selection_changed(UiSelectableButton* button, bool selected)
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

    if (selected)
    {
        set_selected_index(index);
        return;
    }

    if (_selected_index == index)
    {
        _selected_index = -1;
        if (_on_selection_changed)
        {
            _on_selection_changed(_selected_index, nullptr);
        }
    }
}

int UiButtonGroup::button_index(const UiSelectableButton* button) const
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
