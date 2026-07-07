#pragma once

#include "../core/geometry/rect.h"
#include "../core/geometry/vector2.h"

#include <cstdint>
#include <string_view>

namespace elysia::physics
{  
struct Collider
{
    elysia::core::Rect local_rect{};
    elysia::core::Vector2 offset{};

    std::uint32_t layer = 0;
    std::uint32_t mask = 0xffffffffu;

    std::string_view tag{};

    bool enabled = true;
    bool is_trigger = false;
};
}
