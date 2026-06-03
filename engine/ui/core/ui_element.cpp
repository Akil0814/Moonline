#include "ui_element.h"

//#include "../ui_theme_manager.h"

UiElement::UiElement(Vector2 position, Vector2 size, int order)
    : _order(order)
{
    set_position(position);
    set_size(size);
    //UiThemeManager::instance().register_element(this);
}

UiElement::~UiElement()
{
/*    if (UiThemeManager::is_alive())
    {
        UiThemeManager::instance().unregister_element(this);
    }*/
}

void UiElement::reset()
{
    _destroyed = false;
    _visible = true;
}

void UiElement::set_position(const Vector2& position)
{
    _position = position;
    sync_rect();
}

const Vector2& UiElement::position() const
{
    return _position;
}

void UiElement::set_center_pos(const Vector2& center_pos)
{
    _position = {
        center_pos.x - _size.x * 0.5f,
        center_pos.y - _size.y * 0.5f
    };
    sync_rect();
}

Vector2 UiElement::center() const
{
    return {
        _position.x + _size.x * 0.5f,
        _position.y + _size.y * 0.5f
    };
}

void UiElement::set_size(const Vector2& size)
{
    _size = size;
    sync_rect();
}

const Vector2& UiElement::size() const
{
    return _size;
}

const SDL_Rect& UiElement::rect() const
{
    return _rect;
}

int UiElement::order() const
{
    return _order;
}

void UiElement::set_visible(bool visible)
{
    _visible = visible;
}

bool UiElement::is_visible() const
{
    return _visible;
}

void UiElement::destroy()
{
    _destroyed = true;
}

bool UiElement::is_destroyed() const
{
    return _destroyed;
}

void UiElement::set_use_theme(bool use_theme)
{
    _use_theme = use_theme;
}

bool UiElement::uses_theme() const
{
    return _use_theme;
}

void UiElement::sync_rect()
{
    _rect.x = static_cast<int>(_position.x);
    _rect.y = static_cast<int>(_position.y);

    const int width = static_cast<int>(_size.x);
    const int height = static_cast<int>(_size.y);
    _rect.w = width > 0 ? width : 0;
    _rect.h = height > 0 ? height : 0;
}
