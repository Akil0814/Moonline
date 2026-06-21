#pragma once

#include "../core/geometry/rect.h"
#include "../core/geometry/vector2.h"

namespace elysia::camera
{
class Camera
{
public:
    Camera() = default;
    Camera(
        const elysia::core::Vector2& center,
        const elysia::core::Vector2& viewport_size
    ) noexcept;

    void set_center(const elysia::core::Vector2& center) noexcept;
    void set_viewport_size(const elysia::core::Vector2& viewport_size) noexcept;

    [[nodiscard]] const elysia::core::Vector2& center() const noexcept;
    [[nodiscard]] const elysia::core::Vector2& viewport_size() const noexcept;

    [[nodiscard]] elysia::core::Rect view_rect() const noexcept;
    [[nodiscard]] elysia::core::Vector2 world_to_screen(
        const elysia::core::Vector2& world_position
    ) const noexcept;
    [[nodiscard]] elysia::core::Rect world_to_screen(
        const elysia::core::Rect& world_rect
    ) const noexcept;

private:
    elysia::core::Vector2 _center{};
    elysia::core::Vector2 _viewport_size{};
};
}
