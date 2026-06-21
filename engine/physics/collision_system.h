#pragma once

#include <vector>

namespace elysia::physics
{
class CollisionSystem
{
public:
    CollisionSystem() = default;

    template <typename Entry>
    void dispatch_events(const std::vector<Entry>& collider_entries)
    {
        (void)collider_entries;
    }

    void reset();
    void clear();
};
}
