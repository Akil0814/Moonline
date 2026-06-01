#include "ui_element.h"

#include "style/ui_theme_manager.h"

UiElement::UiElement(Vector2 position, Vector2 size, int order)
    : GameObject(DepthLayer::UI, order)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
    UiThemeManager::instance().register_element(this);
}

UiElement::~UiElement()
{
    UiThemeManager::instance().unregister_element(this);
}

void UiElement::reset()
{
    GameObject::reset();
    mark_theme_dirty();
}

void UiElement::set_use_theme(bool use_theme)
{
    _use_theme = use_theme;
    if (_use_theme)
    {
        mark_theme_dirty();
    }
}

bool UiElement::uses_theme() const
{
    return _use_theme;
}

void UiElement::mark_theme_dirty()
{
    _theme_dirty = true;
}

void UiElement::refresh_theme_if_needed()
{
    if (!_use_theme || !_theme_dirty)
    {
        return;
    }

    _theme_dirty = false;
    apply_theme(UiThemeManager::instance().current_theme());
}
