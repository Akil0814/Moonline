#pragma once

#include "../core/geometry/rect.h"
#include "../core/geometry/vector2.h"

#include <cstdint>
#include <string_view>

namespace elysia::physics
{
using ColliderId = std::uint64_t;
using CollisionBits = std::uint32_t;

inline constexpr ColliderId InvalidColliderId = 0;

struct CollisionFilter
{
    CollisionBits category = 0;
    CollisionBits mask = 0xffffffffu;
    std::int32_t group = 0;
};

enum class CollisionResponse : std::uint8_t
{
    Ignore,
    Overlap,
    Block
};

struct Collider
{
    ColliderId id = InvalidColliderId;

    elysia::core::Rect local_rect{};
    elysia::core::Vector2 offset{};

    CollisionFilter filter{};
    CollisionResponse response = CollisionResponse::Block;

    std::string_view tag{};

    bool enabled = true;
    // Retained for source compatibility while CollisionResponse becomes canonical.
    bool is_trigger = false;
};
}
