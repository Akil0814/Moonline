#pragma once

#include "vector2.h"

#include <algorithm>
#include <cmath>

struct Rect
{
    static constexpr float k_epsilon = Vector2::k_epsilon;

    float _x = 0.0f;
    float _y = 0.0f;
    float _width = 0.0f;
    float _height = 0.0f;

    constexpr Rect() noexcept = default;
    constexpr Rect(
        float rect_x,
        float rect_y,
        float rect_width,
        float rect_height
    ) noexcept
        : _x(rect_x), _y(rect_y), _width(rect_width), _height(rect_height) {}

    constexpr Rect(const Vector2& position, const Vector2& size) noexcept
        : _x(position.x), _y(position.y), _width(size.x), _height(size.y) {}

    [[nodiscard]] static constexpr Rect zero() noexcept
    {
        return Rect();
    }

    [[nodiscard]] static constexpr Rect from_position_size(
        const Vector2& position,
        const Vector2& size
    ) noexcept
    {
        return Rect(position, size);
    }

    [[nodiscard]] static constexpr Rect from_center(
        const Vector2& center,
        const Vector2& size
    ) noexcept
    {
        return Rect(
            center.x - size.x * 0.5f,
            center.y - size.y * 0.5f,
            size.x,
            size.y
        );
    }

    [[nodiscard]] constexpr bool operator==(const Rect& rect) const noexcept
    {
        return _x == rect._x
            && _y == rect._y
            && _width == rect._width
            && _height == rect._height;
    }

    [[nodiscard]] constexpr bool operator!=(const Rect& rect) const noexcept
    {
        return !(*this == rect);
    }

    [[nodiscard]] bool nearly_equals(
        const Rect& rect,
        float epsilon = k_epsilon
    ) const noexcept
    {
        return std::fabs(_x - rect._x) <= epsilon
            && std::fabs(_y - rect._y) <= epsilon
            && std::fabs(_width - rect._width) <= epsilon
            && std::fabs(_height - rect._height) <= epsilon;
    }

    [[nodiscard]] constexpr Vector2 position() const noexcept
    {
        return Vector2(_x, _y);
    }

    [[nodiscard]] constexpr Vector2 size() const noexcept
    {
        return Vector2(_width, _height);
    }

    [[nodiscard]] constexpr Vector2 center() const noexcept
    {
        return Vector2(_x + _width * 0.5f, _y + _height * 0.5f);
    }

    void set_position(const Vector2& position) noexcept
    {
        _x = position.x;
        _y = position.y;
    }

    void set_size(const Vector2& size) noexcept
    {
        _width = size.x;
        _height = size.y;
    }

    void set_center(const Vector2& center) noexcept
    {
        _x = center.x - _width * 0.5f;
        _y = center.y - _height * 0.5f;
    }

    [[nodiscard]] constexpr float left() const noexcept
    {
        return _x < _x + _width ? _x : _x + _width;
    }

    [[nodiscard]] constexpr float right() const noexcept
    {
        return _x > _x + _width ? _x : _x + _width;
    }

    [[nodiscard]] constexpr float top() const noexcept
    {
        return _y < _y + _height ? _y : _y + _height;
    }

    [[nodiscard]] constexpr float bottom() const noexcept
    {
        return _y > _y + _height ? _y : _y + _height;
    }

    [[nodiscard]] constexpr float area() const noexcept
    {
        Rect rect = normalized();
        return rect._width * rect._height;
    }

    [[nodiscard]] bool is_empty(float epsilon = k_epsilon) const noexcept
    {
        Rect rect = normalized();
        return rect._width <= epsilon || rect._height <= epsilon;
    }

    [[nodiscard]] constexpr Rect translated(const Vector2& offset) const noexcept
    {
        return Rect(_x + offset.x, _y + offset.y, _width, _height);
    }

    [[nodiscard]] constexpr Rect expanded(float amount) const noexcept
    {
        return Rect(
            _x - amount,
            _y - amount,
            _width + amount * 2.0f,
            _height + amount * 2.0f
        );
    }

    [[nodiscard]] constexpr Rect expanded(const Vector2& amount) const noexcept
    {
        return Rect(
            _x - amount.x,
            _y - amount.y,
            _width + amount.x * 2.0f,
            _height + amount.y * 2.0f
        );
    }

    [[nodiscard]] constexpr Rect normalized() const noexcept
    {
        const float rect_left = left();
        const float rect_top = top();
        return Rect(rect_left, rect_top, right() - rect_left, bottom() - rect_top);
    }

    [[nodiscard]] bool contains(const Vector2& point) const noexcept
    {
        return point.x >= left()
            && point.x <= right()
            && point.y >= top()
            && point.y <= bottom();
    }

    [[nodiscard]] bool contains(const Rect& rect) const noexcept
    {
        return rect.left() >= left()
            && rect.right() <= right()
            && rect.top() >= top()
            && rect.bottom() <= bottom();
    }

    [[nodiscard]] bool intersects(const Rect& rect) const noexcept
    {
        return right() > rect.left()
            && left() < rect.right()
            && bottom() > rect.top()
            && top() < rect.bottom();
    }

    [[nodiscard]] bool touches_or_intersects(const Rect& rect) const noexcept
    {
        return right() >= rect.left()
            && left() <= rect.right()
            && bottom() >= rect.top()
            && top() <= rect.bottom();
    }

    [[nodiscard]] Rect merged(const Rect& rect) const noexcept
    {
        const float merged_left = std::min(left(), rect.left());
        const float merged_top = std::min(top(), rect.top());
        const float merged_right = std::max(right(), rect.right());
        const float merged_bottom = std::max(bottom(), rect.bottom());

        return Rect(
            merged_left,
            merged_top,
            merged_right - merged_left,
            merged_bottom - merged_top
        );
    }

    [[nodiscard]] Rect intersection(const Rect& rect) const noexcept
    {
        const float overlap_left = std::max(left(), rect.left());
        const float overlap_top = std::max(top(), rect.top());
        const float overlap_right = std::min(right(), rect.right());
        const float overlap_bottom = std::min(bottom(), rect.bottom());

        if (overlap_right <= overlap_left || overlap_bottom <= overlap_top)
            return Rect(overlap_left, overlap_top, 0.0f, 0.0f);

        return Rect(
            overlap_left,
            overlap_top,
            overlap_right - overlap_left,
            overlap_bottom - overlap_top
        );
    }
};
