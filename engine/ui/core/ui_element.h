#pragma once

#include "../../core/geometry/vector2.h"

#include <SDL.h>

struct UiTheme;

class UiElement
{
public:
    explicit UiElement(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);
    virtual ~UiElement();

    UiElement(const UiElement&) = delete;
    UiElement& operator=(const UiElement&) = delete;
    UiElement(UiElement&&) = delete;
    UiElement& operator=(UiElement&&) = delete;

    virtual void on_render(SDL_Renderer* renderer) {}

    virtual void reset();

    virtual void set_position(const Vector2& position);
    [[nodiscard]] const Vector2& position() const;

    void set_center_pos(const Vector2& center_pos);
    [[nodiscard]] Vector2 center() const;

    virtual void set_size(const Vector2& size);
    [[nodiscard]] const Vector2& size() const;

    [[nodiscard]] const SDL_Rect& rect() const;
    [[nodiscard]] int order() const;

    void set_visible(bool visible);
    [[nodiscard]] bool is_visible() const;


    void destroy();
    [[nodiscard]] bool is_destroyed() const;

    void set_use_theme(bool use_theme);
    [[nodiscard]] bool uses_theme() const;

protected:
    virtual void apply_theme(const UiTheme& theme) {}

private:
    void sync_rect();

private:
    Vector2 _position = Vector2::zero();
    Vector2 _size = Vector2::zero();
    SDL_Rect _rect{ 0, 0, 0, 0 };

    int _order = 0;

    bool _destroyed = false;
    bool _visible = true;

    bool _use_theme = true;
};
