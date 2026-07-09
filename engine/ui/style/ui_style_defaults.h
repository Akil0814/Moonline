#pragma once

#include "ui_theme.h"
#include "ui_visual_styles.h"

namespace elysia::ui
{
struct UiStyleDefaults
{
    [[nodiscard]] static const UiTheme& theme() noexcept
    {
        return builtin_theme(UiBuiltinTheme::BlueGlassMoon);
    }

    [[nodiscard]] static UiEnabledDisabledColors text() noexcept
    {
        return theme().button(UiButtonThemeRole::Default).text;
    }

    [[nodiscard]] static UiEnabledDisabledColors muted_text() noexcept
    {
        return UiEnabledDisabledColors{
            theme().label(UiLabelThemeRole::Muted).text,
            theme().button(UiButtonThemeRole::Default).text.disabled
        };
    }

    [[nodiscard]] static UiEnabledDisabledColors secondary_text() noexcept
    {
        return theme().text_input_style.text;
    }

    [[nodiscard]] static UiEnabledDisabledColors placeholder_text() noexcept
    {
        return theme().text_input_style.placeholder;
    }

    [[nodiscard]] static UiInteractiveColors interactive_surface() noexcept
    {
        return theme().button(UiButtonThemeRole::Default).chrome.background;
    }

    [[nodiscard]] static UiChromeStyle interactive_chrome() noexcept
    {
        return apply_theme_colors(UiChromeStyle{},theme().button(UiButtonThemeRole::Default).chrome);
    }

    [[nodiscard]] static UiLabelStyle label() noexcept
    {
        return apply_theme_colors(UiLabelStyle{},theme().label(UiLabelThemeRole::Default));
    }

    [[nodiscard]] static UiNumberStyle number() noexcept
    {
        return apply_theme_colors(UiNumberStyle{},theme().number_style);
    }

    [[nodiscard]] static UiBarStyle bar() noexcept
    {
        return apply_theme_colors(UiBarStyle{},theme().bar(UiBarThemeRole::Default));
    }

    [[nodiscard]] static UiPanelStyle panel() noexcept
    {
        return apply_theme_colors(UiPanelStyle{},theme().panel(UiPanelThemeRole::Default));
    }

    [[nodiscard]] static UiChromeContainerStyle chrome_container() noexcept
    {
        return apply_theme_colors(UiChromeContainerStyle{},theme().chrome_container_style);
    }

    [[nodiscard]] static UiWindowStyle window() noexcept
    {
        return apply_theme_colors(UiWindowStyle{},theme().window_style);
    }

    [[nodiscard]] static UiButtonStyle button() noexcept
    {
        return apply_theme_colors(UiButtonStyle{},theme().button(UiButtonThemeRole::Default));
    }

    [[nodiscard]] static UiCheckboxStyle checkbox() noexcept
    {
        return apply_theme_colors(UiCheckboxStyle{},theme().checkbox_style);
    }

    [[nodiscard]] static UiRadioButtonStyle radio_button() noexcept
    {
        return apply_theme_colors(UiRadioButtonStyle{},theme().radio_button_style);
    }

    [[nodiscard]] static UiDragHandleStyle drag_handle() noexcept
    {
        return apply_theme_colors(UiDragHandleStyle{},theme().drag_handle_style);
    }

    [[nodiscard]] static UiSliderStyle slider() noexcept
    {
        return apply_theme_colors(UiSliderStyle{},theme().slider_style);
    }

    [[nodiscard]] static UiTextInputStyle text_input() noexcept
    {
        return apply_theme_colors(UiTextInputStyle{},theme().text_input_style);
    }

    [[nodiscard]] static UiScrollBarStyle scrollbar() noexcept
    {
        return apply_theme_colors(UiScrollBarStyle{},theme().scroll_container_style.scrollbar);
    }

    [[nodiscard]] static UiScrollContainerStyle scroll_container() noexcept
    {
        return apply_theme_colors(UiScrollContainerStyle{},theme().scroll_container_style);
    }

    [[nodiscard]] static UiEnabledDisabledColors labeled_checkbox_text() noexcept
    {
        return UiEnabledDisabledColors{
            theme().label(UiLabelThemeRole::Default).text,
            theme().button(UiButtonThemeRole::Default).text.disabled
        };
    }
};
}
