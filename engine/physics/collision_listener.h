#pragma once

#include "collision_info.h"

namespace elysia::physics
{
class CollisionListener
{
public:
    virtual ~CollisionListener() = default;

    virtual void on_collision_enter(const CollisionInfo& info)
    {
        (void)info;
    }

    virtual void on_collision_stay(const CollisionInfo& info)
    {
        (void)info;
    }

    virtual void on_collision_exit(const CollisionInfo& info)
    {
        (void)info;
    }

    virtual void on_trigger_enter(const CollisionInfo& info)
    {
        (void)info;
    }

    virtual void on_trigger_stay(const CollisionInfo& info)
    {
        (void)info;
    }

    virtual void on_trigger_exit(const CollisionInfo& info)
    {
        (void)info;
    }
};
}
