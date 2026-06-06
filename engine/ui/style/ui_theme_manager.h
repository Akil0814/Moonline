/*#pragma once

#include "ui_theme.h"

#include <vector>

class UiElement;

class UiThemeManager
{
public:
    static UiThemeManager& instance();
    static bool is_alive();

    [[nodiscard]] const UiTheme& current_theme() const;
    void set_theme(const UiTheme& theme);
    void reset_to_default_theme();

    void register_element(UiElement* element);
    void unregister_element(UiElement* element);

private:
    UiThemeManager();
    ~UiThemeManager();
    void mark_registered_elements_dirty();

private:
    UiTheme _current_theme = make_loading_blue_theme();
    std::vector<UiElement*> _registered_elements;
};*/
