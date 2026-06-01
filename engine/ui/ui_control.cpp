#include "ui_control.h"

UiControl::UiControl(Vector2 position, Vector2 size, int order)
    : UiElement(position, size, order)
{
}

void UiControl::reset()
{
    UiElement::reset();
    _enabled = true;
    _is_focused = false;
}

void UiControl::set_enabled(bool enabled)
{
    _enabled = enabled;
    if (!_enabled)
    {
        _is_focused = false;
    }
}

bool UiControl::is_enabled() const
{
    return _enabled;
}

void UiControl::set_focused(bool focused)
{
    _is_focused = _enabled && focused;
}

bool UiControl::is_focused() const
{
    return _is_focused;
}

GameObject* UiControl::game_object()
{
    return this;
}

const GameObject* UiControl::game_object() const
{
    return this;
}
