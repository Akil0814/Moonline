#include "ui_multi_select_button_group.h"

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

    if (!ui_selectable_button_group_utils::add_button(
            _buttons,
            button,
            UiSelectableActivationBehavior::Toggle,
            [this](UiSelectableButton& selectable_button, bool selected)
            {
                handle_button_selection_changed(&selectable_button, selected);
            }
        ))
    {
        return;
    }
}

bool UiMultiSelectButtonGroup::remove_button(const UiSelectableButton* button)
{
    if (!ui_selectable_button_group_utils::remove_button(_buttons, button))
    {
        return false;
    }
    return true;
}

void UiMultiSelectButtonGroup::clear_buttons()
{
    ui_selectable_button_group_utils::clear_registrations(_buttons);
}

void UiMultiSelectButtonGroup::clear_selection()
{
    _is_syncing_selection = true;
    for (UiSelectableButtonRegistration& registration : _buttons)
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
    for (const UiSelectableButtonRegistration& registration : _buttons)
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
    ui_selectable_button_group_utils::cleanup_registrations(_buttons);
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
    return ui_selectable_button_group_utils::button_index(_buttons, button);
}

std::shared_ptr<UiSelectableButton> UiMultiSelectButtonGroup::button_at(int index) const
{
    return ui_selectable_button_group_utils::button_at(_buttons, index);
}
