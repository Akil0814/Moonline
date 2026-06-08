#pragma once

#include "../core/game_object.h"
#include "../ui/core/ui_element.h"

#include <algorithm>
#include <functional>
#include <tuple>
#include <vector>

namespace scene_input_order
{
[[nodiscard]] inline bool receiver_priority_less(
    const SceneObject* lhs,
    const SceneObject* rhs
) noexcept
{
    if (lhs == rhs)
    {
        return false;
    }

    const UiElement* lhs_ui = dynamic_cast<const UiElement*>(lhs);
    const UiElement* rhs_ui = dynamic_cast<const UiElement*>(rhs);

    if (lhs_ui && rhs_ui)
    {
        return lhs_ui->order() > rhs_ui->order();
    }

    const GameObject* lhs_game = dynamic_cast<const GameObject*>(lhs);
    const GameObject* rhs_game = dynamic_cast<const GameObject*>(rhs);

    if (lhs_game && rhs_game)
    {
        return std::make_tuple(
            lhs_game->depth_layer(),
            lhs_game->order_in_layer()
        ) > std::make_tuple(
            rhs_game->depth_layer(),
            rhs_game->order_in_layer()
        );
    }

    if (lhs_ui && rhs_game)
    {
        return true;
    }

    if (lhs_game && rhs_ui)
    {
        return false;
    }

    return std::less<const SceneObject*>{}(lhs, rhs);
}

template <typename Entry>
void insert_receiver_entry_sorted(std::vector<Entry>& entries, Entry entry)
{
    const auto iter = std::upper_bound(
        entries.begin(),
        entries.end(),
        entry,
        [](const Entry& lhs, const Entry& rhs)
        {
            return receiver_priority_less(lhs.object, rhs.object);
        }
    );

    entries.insert(iter, entry);
}
} // namespace scene_input_order
