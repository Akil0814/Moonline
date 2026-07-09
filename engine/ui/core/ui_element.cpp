#include "ui_element.h"

#include "../style/ui_theme_manager.h"

#include <algorithm>

namespace elysia::ui
{
UiElement::~UiElement()
{
    detach_all_theme_managers();
}

void UiElement::set_use_theme(bool use_theme) noexcept
{
    if (_use_theme == use_theme)
        return;

    _use_theme = use_theme;
    if (!_use_theme)
        return;

    request_theme_reapply();
}

void UiElement::request_theme_reapply() noexcept
{
    for (UiThemeManager* manager : _theme_managers)
    {
        if (manager)
            manager->apply_theme_to_element(*this);
    }
}

void UiElement::attach_theme_manager(UiThemeManager& manager)
{
    const auto found = std::find(_theme_managers.begin(),_theme_managers.end(),&manager);
    if (found == _theme_managers.end())
        _theme_managers.push_back(&manager);
}

void UiElement::detach_theme_manager(UiThemeManager& manager) noexcept
{
    _theme_managers.erase(std::remove(_theme_managers.begin(),_theme_managers.end(),&manager),_theme_managers.end());
}

void UiElement::detach_all_theme_managers() noexcept
{
    std::vector<UiThemeManager*> managers = _theme_managers;
    for (UiThemeManager* manager : managers)
    {
        if (manager)
            manager->invalidate_element_registration(*this);
    }
    _theme_managers.clear();
}
}
