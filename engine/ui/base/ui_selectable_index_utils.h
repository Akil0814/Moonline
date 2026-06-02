#pragma once

#include <algorithm>
#include <cstddef>

namespace ui_selectable_index_utils
{
    template <typename EnabledPredicate>
    int resolve_index(int index, size_t item_count, EnabledPredicate&& is_enabled)
    {
        if (item_count == 0)
        {
            return -1;
        }

        int resolved_index = std::clamp(index, 0, static_cast<int>(item_count) - 1);
        const int start_index = resolved_index;
        do
        {
            if (is_enabled(resolved_index))
            {
                return resolved_index;
            }

            resolved_index = (resolved_index + 1) % static_cast<int>(item_count);
        } while (resolved_index != start_index);

        return -1;
    }
}
