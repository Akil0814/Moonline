#include "ui_theme_manager.h"

#include "../base/ui_element.h"

#include <algorithm>

namespace
{
    bool& ui_theme_manager_alive_flag()
    {
        static bool alive = false;
        return alive;
    }
}

UiThemeManager& UiThemeManager::instance()
{
    static UiThemeManager instance;
    return instance;
}

UiThemeManager::UiThemeManager()
{
    ui_theme_manager_alive_flag() = true;
}

UiThemeManager::~UiThemeManager()
{
    ui_theme_manager_alive_flag() = false;
}

bool UiThemeManager::is_alive()
{
    return ui_theme_manager_alive_flag();
}

const UiTheme& UiThemeManager::current_theme() const
{
    return _current_theme;
}

void UiThemeManager::set_theme(const UiTheme& theme)
{
    _current_theme = theme;
    mark_registered_elements_dirty();
}

void UiThemeManager::reset_to_default_theme()
{
    set_theme(make_loading_blue_theme());
}

void UiThemeManager::register_element(UiElement* element)
{
    if (!element)
    {
        return;
    }

    const auto found = std::find(_registered_elements.begin(), _registered_elements.end(), element);
    if (found == _registered_elements.end())
    {
        _registered_elements.push_back(element);
    }
}

void UiThemeManager::unregister_element(UiElement* element)
{
    _registered_elements.erase(
        std::remove(_registered_elements.begin(), _registered_elements.end(), element),
        _registered_elements.end()
    );
}

void UiThemeManager::mark_registered_elements_dirty()
{
    _registered_elements.erase(
        std::remove(_registered_elements.begin(), _registered_elements.end(), nullptr),
        _registered_elements.end()
    );

    for (UiElement* element : _registered_elements)
    {
        if (element)
        {
            element->mark_theme_dirty();
        }
    }
}
