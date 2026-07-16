#pragma once

#include "collider.h"

#include "../core/geometry/rect.h"
#include "../core/geometry/vector2.h"

namespace elysia::physics
{
struct CollisionPair
{
    ColliderId first = InvalidColliderId;
    ColliderId second = InvalidColliderId;
};

struct CollisionOverlap
{
    CollisionPair pair{};
    elysia::core::Rect overlap_rect{};
};

struct CollisionContact
{
    CollisionPair pair{};
    elysia::core::Rect overlap_rect{};
    elysia::core::Vector2 normal{};
    CollisionResponse response = CollisionResponse::Ignore;
};
}
