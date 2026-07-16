#pragma once

#include "camera.h"
#include "camera_effect.h"
#include "follow_strategy.h"

#include <memory>
#include <optional>

namespace elysia::camera
{
class CameraController
{
public:
    explicit CameraController(Camera& camera) noexcept;

    void set_follow_strategy(std::unique_ptr<IFollowStrategy> follow_strategy) noexcept;
    void set_focus_rect(std::optional<elysia::core::Rect> focus_rect) noexcept;
    void set_world_bounds(std::optional<elysia::core::Rect> world_bounds) noexcept;
    void set_viewport_size(const elysia::core::Vector2& viewport_size) noexcept;
    void set_center(const elysia::core::Vector2& center) noexcept;

    void snap_to_focus() noexcept;
    void start_shake(const CameraShakeParams& params);
    void clear_shake() noexcept;
    void reset_scene_state() noexcept;
    void update(double delta_seconds);

    [[nodiscard]] Camera& camera() noexcept;
    [[nodiscard]] const Camera& camera() const noexcept;
    [[nodiscard]] const IFollowStrategy* follow_strategy() const noexcept;
    [[nodiscard]] const std::optional<elysia::core::Rect>& focus_rect() const noexcept;
    [[nodiscard]] const std::optional<elysia::core::Rect>& world_bounds() const noexcept;
    [[nodiscard]] const elysia::core::Vector2& logical_center() const noexcept;
    [[nodiscard]] const elysia::core::Vector2& final_render_center() const noexcept;

private:
    [[nodiscard]] elysia::core::Vector2 clamp_center_to_world_bounds(
        const elysia::core::Vector2& center
    ) const noexcept;
    void write_final_camera_center(const elysia::core::Vector2& center) noexcept;

private:
    Camera& _camera;
    elysia::core::Vector2 _logical_center{};
    elysia::core::Vector2 _final_render_center{};
    std::unique_ptr<IFollowStrategy> _follow_strategy;
    std::optional<elysia::core::Rect> _focus_rect;
    std::optional<elysia::core::Rect> _world_bounds;
    std::unique_ptr<CameraEffect> _active_effect;
    bool _has_initialized_focus = false;
};
}
