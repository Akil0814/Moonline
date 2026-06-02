#pragma once

#include "ui_selectable_button.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

struct UiSelectableButtonRegistration
{
    std::weak_ptr<UiSelectableButton> _button;
    UiSelectableButtonListenerId _listener_id = 0;
};

namespace ui_selectable_button_group_utils
{
    inline void clear_registrations(std::vector<UiSelectableButtonRegistration>& buttons)
    {
        for (UiSelectableButtonRegistration& registration : buttons)
        {
            const std::shared_ptr<UiSelectableButton> button = registration._button.lock();
            if (button)
            {
                button->remove_selection_listener(registration._listener_id);
            }
        }

        buttons.clear();
    }

    inline void cleanup_registrations(std::vector<UiSelectableButtonRegistration>& buttons)
    {
        buttons.erase(
            std::remove_if(
                buttons.begin(),
                buttons.end(),
                [](const UiSelectableButtonRegistration& registration)
                {
                    return registration._button.expired();
                }
            ),
            buttons.end()
        );
    }

    inline int button_index(
        const std::vector<UiSelectableButtonRegistration>& buttons,
        const UiSelectableButton* button
    )
    {
        if (!button)
        {
            return -1;
        }

        for (int index = 0; index < static_cast<int>(buttons.size()); ++index)
        {
            const std::shared_ptr<UiSelectableButton> current_button = buttons[static_cast<size_t>(index)]._button.lock();
            if (current_button.get() == button)
            {
                return index;
            }
        }

        return -1;
    }

    inline std::shared_ptr<UiSelectableButton> button_at(
        const std::vector<UiSelectableButtonRegistration>& buttons,
        int index
    )
    {
        if (index < 0 || index >= static_cast<int>(buttons.size()))
        {
            return nullptr;
        }

        return buttons[static_cast<size_t>(index)]._button.lock();
    }

    inline bool add_button(
        std::vector<UiSelectableButtonRegistration>& buttons,
        const std::shared_ptr<UiSelectableButton>& button,
        UiSelectableActivationBehavior activation_behavior,
        const std::function<void(UiSelectableButton&, bool)>& on_selection_changed
    )
    {
        if (!button)
        {
            return false;
        }

        cleanup_registrations(buttons);
        if (button_index(buttons, button.get()) >= 0)
        {
            return false;
        }

        UiSelectableButtonRegistration registration;
        registration._button = button;
        button->set_activation_behavior(activation_behavior);
        registration._listener_id = button->add_selection_listener(on_selection_changed);
        buttons.push_back(std::move(registration));
        return true;
    }

    inline bool remove_button(
        std::vector<UiSelectableButtonRegistration>& buttons,
        const UiSelectableButton* button
    )
    {
        cleanup_registrations(buttons);
        const int index = button_index(buttons, button);
        if (index < 0)
        {
            return false;
        }

        const std::shared_ptr<UiSelectableButton> button_handle = buttons[static_cast<size_t>(index)]._button.lock();
        if (button_handle)
        {
            button_handle->remove_selection_listener(buttons[static_cast<size_t>(index)]._listener_id);
        }

        buttons.erase(buttons.begin() + index);
        return true;
    }
}
