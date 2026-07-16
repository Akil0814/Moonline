#pragma once

#include "actor_collision_rig.h"
#include "gameplay_collision_types.h"

namespace elysia::gameplay::collision
{
class GameplayCollisionService
{
public:
    virtual ~GameplayCollisionService() = default;

    virtual void bind_actor(const ActorCollisionRig& rig) = 0;
    virtual void bind_collider(const ColliderBinding& binding) = 0;
    virtual void bind_hit_box(const HitBoxBinding& binding) = 0;
    virtual void unbind_collider(elysia::physics::ColliderId collider) = 0;
    virtual void clear() noexcept = 0;
};
}
