#pragma once

#include <cstdint>
#include <vector>

#include "../../core/scene_object.h"
#include "../../core/geometry/vector2.h"
#include "../../core/geometry/rect.h"
#include "../../core/render/render_command.h"

namespace elysia::ui
{
struct UiTheme;
class UiChildHost;
// Tag type for constructors that interpret the supplied position as a center point.
struct UiFromCenterTag
{
    explicit constexpr UiFromCenterTag() noexcept = default;
};

inline constexpr UiFromCenterTag from_center{};

class UiElement : public elysia::core::SceneObject
{
public:

    // Higher UI order values are rendered on top and receive input first.
    explicit UiElement(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) 
        noexcept : _screen_rect(rect), _order(order) {}

    UiElement( const elysia::core::Vector2& position, const elysia::core::Vector2& size, int order = 0)
        noexcept : _screen_rect(position, size), _order(order) {}

    UiElement(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag, int order = 0 )
        noexcept : _screen_rect(elysia::core::Rect::from_center(center, size)), _order(order) {}

    virtual ~UiElement() = default;

    UiElement(const UiElement&) = delete;
    UiElement& operator=(const UiElement&) = delete;
    UiElement(UiElement&&) = delete;
    UiElement& operator=(UiElement&&) = delete;

    // Appends this element's draw commands in local visual order.
    virtual void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
    {
        (void)out_commands;
    }

    // Reports the size this element contributes to parent layout calculations.
    [[nodiscard]] virtual elysia::core::Vector2 content_extent() const noexcept
    {
        return size();
    }

    void reset() noexcept override
    {
        elysia::core::SceneObject::reset();
        _layout_parent = nullptr;
        _use_theme = false;
        _opacity = 255;
    }

    // Updates the screen-space bounds and invalidates intrinsic layout when the size changes.
    void set_screen_rect(const elysia::core::Rect& rect) noexcept
    {
        if (_screen_rect.nearly_equals(rect))
            return;

        const bool size_changed = !_screen_rect.size().nearly_equals(rect.size());
        _screen_rect = rect;
        if (size_changed)
            notify_layout_parent_of_intrinsic_layout_invalidation();
    }

    void set_position(const elysia::core::Vector2& position) noexcept
    {
        if (_screen_rect.position().nearly_equals(position))
            return;
        _screen_rect.set_position(position);
    }

    void set_center(const elysia::core::Vector2& center) noexcept
    {
        if (_screen_rect.center().nearly_equals(center))
            return;
        _screen_rect.set_center(center);
    }

    void set_size(const elysia::core::Vector2& size) noexcept
    {
        if (_screen_rect.size().nearly_equals(size))
            return;
        elysia::core::Rect next_rect = _screen_rect;
        next_rect.set_size(size);
        set_screen_rect(next_rect);
    }

    [[nodiscard]] const elysia::core::Rect& screen_rect() const noexcept { return _screen_rect; }
    [[nodiscard]] elysia::core::Vector2 position() const noexcept { return _screen_rect.position(); }
    [[nodiscard]] elysia::core::Vector2 center() const noexcept { return _screen_rect.center(); }
    [[nodiscard]] elysia::core::Vector2 size() const noexcept { return _screen_rect.size(); }

    void set_order(int order) noexcept { _order = order; }
    [[nodiscard]] int order() const noexcept { return _order; }

    void set_use_theme(bool use_theme) noexcept { _use_theme = use_theme; }
    [[nodiscard]] bool uses_theme() const noexcept { return _use_theme; }

    void set_opacity(std::uint8_t opacity) noexcept { _opacity = opacity; }
    [[nodiscard]] std::uint8_t opacity() const noexcept { return _opacity; }

    [[nodiscard]] bool update_when_paused() const override{ return true;}

    [[nodiscard]] bool receive_input_when_paused() const override{ return true;}

protected:
    // Applies theme-driven visuals when the element opts into theme ownership.
    virtual void apply_theme(const UiTheme& theme) { (void)theme; }
    // Tells the owning layout host that this element's intrinsic size may have changed.
    void notify_layout_parent_of_intrinsic_layout_invalidation() noexcept;

    void apply_opacity(elysia::core::UiRenderCommand& command) const noexcept
    {
        switch (command.type)
        {
        case elysia::core::UiRenderCommandType::Texture:
            command.alpha = multiply_alpha(command.alpha, _opacity);
            break;
        case elysia::core::UiRenderCommandType::FillRect:
        case elysia::core::UiRenderCommandType::DrawRect:
        case elysia::core::UiRenderCommandType::DrawLine:
        case elysia::core::UiRenderCommandType::FillCircle:
        case elysia::core::UiRenderCommandType::DrawCircle:
            command.color = apply_opacity(command.color);
            break;
        default:
            break;
        }
    }

    [[nodiscard]] elysia::core::Color apply_opacity(elysia::core::Color color) const noexcept
    {
        color.a = multiply_alpha(color.a, _opacity);
        return color;
    }

private:
    friend class UiChildHost;

    static std::uint8_t multiply_alpha(std::uint8_t a, std::uint8_t b) noexcept
    {
        return static_cast<std::uint8_t>((static_cast<unsigned int>(a) * static_cast<unsigned int>(b)) / 255U);
    }

    void set_layout_parent(UiChildHost* parent) noexcept { _layout_parent = parent; }

    elysia::core::Rect _screen_rect{};
    UiChildHost* _layout_parent = nullptr;
    int _order = 0;
    bool _use_theme = false;
    std::uint8_t _opacity = 255;
};
}
