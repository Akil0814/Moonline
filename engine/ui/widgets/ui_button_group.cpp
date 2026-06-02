#include "ui_button_group.h"

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

    if (!ui_selectable_button_group_utils::add_button(
            _buttons,
            button,
            UiSelectableActivationBehavior::Select,
            [this](UiSelectableButton& selectable_button, bool selected)
            {
                handle_button_selection_changed(&selectable_button, selected);
            }
        ))
    {
        return;
    }

    if (button->is_selected())
    {
        set_selected_index(static_cast<int>(_buttons.size()) - 1);
    }
}

bool UiButtonGroup::remove_button(const UiSelectableButton* button)
{
    const int index = button_index(button);
    if (index < 0 || !ui_selectable_button_group_utils::remove_button(_buttons, button))
    {
        return false;
    }
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
    ui_selectable_button_group_utils::clear_registrations(_buttons);
    _selected_index = -1;
}

size_t UiButtonGroup::button_count() const
{
    return _buttons.size();
}

void UiButtonGroup::set_selected_index(int index, bool notify)
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
        for (UiSelectableButtonRegistration& registration : _buttons)
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

    if (notify && _on_selection_changed)
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
    ui_selectable_button_group_utils::cleanup_registrations(_buttons);

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
    return ui_selectable_button_group_utils::button_index(_buttons, button);
}
