#pragma once

#include <memory>
#include <utility>
#include <vector>

template <typename ButtonT, typename IsItemEnabled>
inline void ui_sync_selectable_button_state(
    const std::vector<std::shared_ptr<ButtonT>>& buttons,
    int selected_index,
    bool owner_enabled,
    bool owner_focused,
    IsItemEnabled&& is_item_enabled
)
{
    for (int index = 0; index < static_cast<int>(buttons.size()); ++index)
    {
        const std::shared_ptr<ButtonT>& button = buttons[static_cast<size_t>(index)];
        if (!button)
        {
            continue;
        }

        const bool enabled = owner_enabled && is_item_enabled(index);
        button->set_enabled(enabled);
        button->set_focused(enabled && owner_focused && index == selected_index);
    }
}
