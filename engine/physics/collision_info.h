#pragma once

#include "collider.h"

#include "../core/geometry/rect.h"
#include "../core/geometry/vector2.h"

namespace elysia::core
{
class SceneObject;
}

namespace elysia::physics
{
struct CollisionInfo
{
    elysia::core::SceneObject* self = nullptr;
    elysia::core::SceneObject* other = nullptr;

    const Collider* self_collider = nullptr;
    const Collider* other_collider = nullptr;

    elysia::core::Rect overlap_rect{};
    elysia::core::Vector2 normal{};

    bool is_trigger = false;
};
}
