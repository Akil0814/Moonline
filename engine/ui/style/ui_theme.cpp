#include "ui_theme.h"

#include "ui_palette.h"

namespace elysia::ui
{
namespace
{
template<class Enum>
[[nodiscard]] constexpr std::size_t to_index(Enum value) noexcept
{
    return static_cast<std::size_t>(value);
}

[[nodiscard]] UiEnabledDisabledColors make_text_colors(
    elysia::core::Color enabled,
    elysia::core::Color disabled = UiPalette::text_disabled
) noexcept
{
    return UiEnabledDisabledColors{ enabled,disabled };
}

[[nodiscard]] UiInteractiveColors make_surface_colors(
    elysia::core::Color idle,
    elysia::core::Color focused,
    elysia::core::Color active,
    elysia::core::Color disabled
) noexcept
{
    return UiInteractiveColors{ idle,focused,active,disabled };
}

[[nodiscard]] UiChromeStyle make_chrome(
    const UiInteractiveColors& background,
    const UiEnabledDisabledColors& border,
    bool draw_background = true,
    bool draw_border = true
) noexcept
{
    UiChromeStyle style;
    style.background = background;
    style.border = border;
    style.draw_background = draw_background;
    style.draw_border = draw_border;
    return style;
}

[[nodiscard]] UiTheme make_blue_glass_moon_theme() noexcept
{
    UiTheme theme;

    const UiEnabledDisabledColors text = make_text_colors(UiPalette::text_primary);
    const UiEnabledDisabledColors muted_text = make_text_colors(UiPalette::text_muted);
    const UiEnabledDisabledColors secondary_text = make_text_colors(UiPalette::text_secondary);
    const UiEnabledDisabledColors placeholder_text = make_text_colors(UiPalette::text_placeholder);
    const UiEnabledDisabledColors border = UiEnabledDisabledColors{
        UiPalette::border_default,
        UiPalette::border_disabled
    };
    const UiInteractiveColors interactive_surface = make_surface_colors(
        UiPalette::surface_interactive_idle,
        UiPalette::surface_interactive_focused,
        UiPalette::surface_interactive_active,
        UiPalette::surface_disabled);
    const UiChromeStyle interactive_chrome = make_chrome(interactive_surface,border,true,true);

    theme.label_styles[to_index(UiLabelThemeRole::Default)] = UiLabelStyle{
        UiPalette::text_primary,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Title)] = UiLabelStyle{
        elysia::core::colors::glacial_white,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Subtitle)] = UiLabelStyle{
        UiPalette::text_secondary,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Muted)] = UiLabelStyle{
        UiPalette::text_muted,
        elysia::core::colors::transparent,
        false
    };

    theme.number_style = UiNumberStyle{
        UiPalette::text_primary,
        elysia::core::colors::transparent,
        false
    };

    theme.bar_styles[to_index(UiBarThemeRole::Default)] = UiBarStyle{
        UiPalette::surface_elevated,
        UiPalette::accent_fill,
        UiPalette::border_default,
        false
    };
    theme.bar_styles[to_index(UiBarThemeRole::Progress)] = UiBarStyle{
        UiPalette::surface_elevated,
        UiPalette::accent_fill,
        UiPalette::border_default,
        false
    };

    theme.panel_styles[to_index(UiPanelThemeRole::Default)] = UiPanelStyle{
        false,
        false,
        UiPalette::surface_elevated,
        UiPalette::border_default
    };
    theme.panel_styles[to_index(UiPanelThemeRole::Screen)] = UiPanelStyle{
        false,
        false,
        UiPalette::surface_base,
        UiPalette::border_default
    };
    theme.panel_styles[to_index(UiPanelThemeRole::Dialog)] = UiPanelStyle{
        true,
        true,
        UiPalette::surface_elevated,
        UiPalette::border_focus
    };
    theme.panel_styles[to_index(UiPanelThemeRole::List)] = UiPanelStyle{
        true,
        false,
        UiPalette::surface_elevated,
        UiPalette::border_default
    };

    theme.window_style = UiWindowStyle{
        false,
        false,
        UiPalette::surface_base,
        UiPalette::border_focus
    };

    theme.chrome_container_style = UiChromeContainerStyle{
        true,
        true,
        true,
        UiPalette::surface_elevated,
        UiPalette::border_default,
        UiPalette::surface_base
    };

    theme.button_styles[to_index(UiButtonThemeRole::Default)] = UiButtonStyle{
        interactive_chrome,
        text
    };
    theme.button_styles[to_index(UiButtonThemeRole::Primary)] = UiButtonStyle{
        make_chrome(
            make_surface_colors(
                elysia::core::colors::royal_blue,
                elysia::core::colors::blue_500,
                elysia::core::colors::deep_cobalt_blue,
                UiPalette::surface_disabled),
            UiEnabledDisabledColors{ elysia::core::colors::alice_blue,UiPalette::border_disabled }),
        text
    };
    theme.button_styles[to_index(UiButtonThemeRole::Danger)] = UiButtonStyle{
        make_chrome(
            make_surface_colors(
                elysia::core::colors::red_700,
                elysia::core::colors::red_500,
                elysia::core::colors::red_300,
                UiPalette::surface_disabled),
            UiEnabledDisabledColors{ elysia::core::colors::red_300,UiPalette::border_disabled }),
        text
    };

    theme.checkbox_style = UiCheckboxStyle{
        interactive_chrome,
        UiEnabledDisabledColors{ UiPalette::accent_fill,UiPalette::text_disabled },
        UiCheckboxMarkStyle::Checkmark
    };

    theme.radio_button_style = UiRadioButtonStyle{
        interactive_chrome,
        UiEnabledDisabledColors{ UiPalette::accent_fill,UiPalette::text_disabled },
        text
    };

    theme.drag_handle_style = UiDragHandleStyle{
        elysia::core::Vector2{ 18.0f,18.0f },
        std::nullopt,
        interactive_chrome
    };

    theme.slider_style = UiSliderStyle{
        UiChromeStyle{ interactive_surface,border,false,false },
        UiEnabledDisabledColors{ UiPalette::accent_fill,UiPalette::text_disabled },
        text,
        theme.drag_handle_style
    };

    theme.text_input_style = UiTextInputStyle{
        interactive_chrome,
        secondary_text,
        placeholder_text,
        UiPalette::caret
    };

    theme.scroll_container_style = UiScrollContainerStyle{
        UiScrollBarStyle{
            10.0f,
            4.0f,
            24.0f,
            true,
            UiPalette::scrollbar_track_idle,
            UiPalette::scrollbar_track_focused,
            UiPalette::scrollbar_track_active,
            UiPalette::scrollbar_track_disabled,
            UiPalette::scrollbar_thumb_idle,
            UiPalette::scrollbar_thumb_focused,
            UiPalette::scrollbar_thumb_active,
            UiPalette::scrollbar_thumb_disabled
        },
        true,
        UiPalette::surface_elevated,
        false,
        UiPalette::border_default
    };

    (void)muted_text;
    return theme;
}

[[nodiscard]] UiTheme make_elysia_light_theme() noexcept
{
    UiTheme theme;

    const UiEnabledDisabledColors text = make_text_colors(
        elysia::core::colors::elysia_plum,
        elysia::core::colors::gray_500);
    const UiEnabledDisabledColors muted_text = make_text_colors(
        elysia::core::colors::royal_blue,
        elysia::core::colors::gray_500);
    const UiEnabledDisabledColors placeholder_text = make_text_colors(elysia::core::colors::gray_500,elysia::core::colors::gray_300);
    const UiEnabledDisabledColors secondary_text = make_text_colors(
        elysia::core::colors::royal_blue,
        elysia::core::colors::gray_500);
    const UiEnabledDisabledColors border = UiEnabledDisabledColors{
        elysia::core::colors::elysia_rose_pink,
        elysia::core::colors::gray_300
    };
    const UiInteractiveColors interactive_surface = make_surface_colors(
        elysia::core::colors::elysia_petal_pink,
        elysia::core::colors::elysia_rose_pink,
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::gray_300);
    const UiChromeStyle interactive_chrome = make_chrome(interactive_surface,border,true,true);

    theme.label_styles[to_index(UiLabelThemeRole::Default)] = UiLabelStyle{
        elysia::core::colors::elysia_plum,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Title)] = UiLabelStyle{
        elysia::core::colors::elysia_rose_pink,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Subtitle)] = UiLabelStyle{
        elysia::core::colors::royal_blue,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Muted)] = UiLabelStyle{
        elysia::core::colors::gray_500,
        elysia::core::colors::transparent,
        false
    };

    theme.number_style = UiNumberStyle{
        elysia::core::colors::elysia_plum,
        elysia::core::colors::transparent,
        false
    };

    theme.bar_styles[to_index(UiBarThemeRole::Default)] = UiBarStyle{
        elysia::core::colors::elysia_silk_white,
        elysia::core::colors::elysia_rose_pink,
        elysia::core::colors::elysia_rose_pink,
        false
    };
    theme.bar_styles[to_index(UiBarThemeRole::Progress)] = UiBarStyle{
        elysia::core::colors::elysia_silk_white,
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::elysia_rose_pink,
        false
    };

    theme.panel_styles[to_index(UiPanelThemeRole::Default)] = UiPanelStyle{
        false,
        false,
        elysia::core::colors::elysia_silk_white,
        elysia::core::colors::elysia_rose_pink
    };
    theme.panel_styles[to_index(UiPanelThemeRole::Screen)] = UiPanelStyle{
        true,
        false,
        elysia::core::colors::elysia_blush_white,
        elysia::core::colors::elysia_petal_pink
    };
    theme.panel_styles[to_index(UiPanelThemeRole::Dialog)] = UiPanelStyle{
        true,
        true,
        elysia::core::colors::elysia_silk_white,
        elysia::core::colors::elysia_lilac
    };
    theme.panel_styles[to_index(UiPanelThemeRole::List)] = UiPanelStyle{
        true,
        true,
        elysia::core::colors::elysia_blush_white,
        elysia::core::colors::elysia_petal_pink
    };

    theme.window_style = UiWindowStyle{
        true,
        true,
        elysia::core::colors::elysia_blush_white,
        elysia::core::colors::elysia_lilac
    };

    theme.chrome_container_style = UiChromeContainerStyle{
        true,
        true,
        true,
        elysia::core::colors::elysia_silk_white,
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::elysia_petal_pink
    };

    theme.button_styles[to_index(UiButtonThemeRole::Default)] = UiButtonStyle{
        interactive_chrome,
        text
    };
    theme.button_styles[to_index(UiButtonThemeRole::Primary)] = UiButtonStyle{
        make_chrome(
            make_surface_colors(
                elysia::core::colors::elysia_rose_pink,
                elysia::core::colors::elysia_glow_pink,
                elysia::core::colors::elysia_lilac,
                elysia::core::colors::gray_300),
            UiEnabledDisabledColors{ elysia::core::colors::elysia_plum,elysia::core::colors::gray_300 }),
        make_text_colors(elysia::core::colors::elysia_silk_white,elysia::core::colors::gray_100)
    };
    theme.button_styles[to_index(UiButtonThemeRole::Danger)] = UiButtonStyle{
        make_chrome(
            make_surface_colors(
                elysia::core::colors::pink_500,
                elysia::core::colors::elysia_glow_pink,
                elysia::core::colors::pink_700,
                elysia::core::colors::gray_300),
            UiEnabledDisabledColors{ elysia::core::colors::pink_700,elysia::core::colors::gray_300 }),
        make_text_colors(elysia::core::colors::elysia_silk_white,elysia::core::colors::gray_100)
    };

    theme.checkbox_style = UiCheckboxStyle{
        interactive_chrome,
        UiEnabledDisabledColors{ elysia::core::colors::elysia_rose_pink,elysia::core::colors::gray_500 },
        UiCheckboxMarkStyle::Checkmark
    };

    theme.radio_button_style = UiRadioButtonStyle{
        interactive_chrome,
        UiEnabledDisabledColors{ elysia::core::colors::elysia_rose_pink,elysia::core::colors::gray_500 },
        text
    };

    theme.drag_handle_style = UiDragHandleStyle{
        elysia::core::Vector2{ 18.0f,18.0f },
        std::nullopt,
        interactive_chrome
    };

    theme.slider_style = UiSliderStyle{
        UiChromeStyle{ interactive_surface,border,false,false },
        UiEnabledDisabledColors{ elysia::core::colors::elysia_rose_pink,elysia::core::colors::gray_500 },
        text,
        theme.drag_handle_style
    };

    theme.text_input_style = UiTextInputStyle{
        interactive_chrome,
        secondary_text,
        placeholder_text,
        elysia::core::colors::elysia_lilac
    };

    theme.scroll_container_style = UiScrollContainerStyle{
        UiScrollBarStyle{
            10.0f,
            4.0f,
            24.0f,
            true,
            elysia::core::colors::elysia_blush_white,
            elysia::core::colors::elysia_petal_pink,
            elysia::core::colors::elysia_rose_pink,
            elysia::core::colors::gray_300,
            elysia::core::colors::elysia_rose_pink,
            elysia::core::colors::elysia_lilac,
            elysia::core::colors::elysia_glow_pink,
            elysia::core::colors::gray_500
        },
        true,
        elysia::core::colors::elysia_silk_white,
        true,
        elysia::core::colors::elysia_rose_pink
    };

    (void)muted_text;
    return theme;
}

[[nodiscard]] UiTheme make_elysia_dark_theme() noexcept
{
    UiTheme theme;

    const UiEnabledDisabledColors text = make_text_colors(
        elysia::core::colors::frosted_white,
        elysia::core::colors::steel_blue);
    const UiEnabledDisabledColors secondary_text = make_text_colors(
        elysia::core::colors::alice_blue,
        elysia::core::colors::steel_blue);
    const UiEnabledDisabledColors placeholder_text = make_text_colors(
        elysia::core::colors::steel_blue,
        elysia::core::colors::gray_500);
    const UiEnabledDisabledColors border = UiEnabledDisabledColors{
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::steel_blue
    };
    const UiInteractiveColors interactive_surface = make_surface_colors(
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_mirage_blue,
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::gray_700);
    const UiChromeStyle interactive_chrome = make_chrome(interactive_surface,border,true,true);

    theme.label_styles[to_index(UiLabelThemeRole::Default)] = UiLabelStyle{
        elysia::core::colors::frosted_white,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Title)] = UiLabelStyle{
        elysia::core::colors::elysia_glow_pink,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Subtitle)] = UiLabelStyle{
        elysia::core::colors::alice_blue,
        elysia::core::colors::transparent,
        false
    };
    theme.label_styles[to_index(UiLabelThemeRole::Muted)] = UiLabelStyle{
        elysia::core::colors::steel_blue,
        elysia::core::colors::transparent,
        false
    };

    theme.number_style = UiNumberStyle{
        elysia::core::colors::frosted_white,
        elysia::core::colors::transparent,
        false
    };

    theme.bar_styles[to_index(UiBarThemeRole::Default)] = UiBarStyle{
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::elysia_lilac,
        false
    };
    theme.bar_styles[to_index(UiBarThemeRole::Progress)] = UiBarStyle{
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_glow_pink,
        elysia::core::colors::elysia_lilac,
        false
    };

    theme.panel_styles[to_index(UiPanelThemeRole::Default)] = UiPanelStyle{
        true,
        true,
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_lilac
    };
    theme.panel_styles[to_index(UiPanelThemeRole::Screen)] = UiPanelStyle{
        true,
        false,
        elysia::core::colors::elysia_deep_sea,
        elysia::core::colors::elysia_mirage_blue
    };
    theme.panel_styles[to_index(UiPanelThemeRole::Dialog)] = UiPanelStyle{
        true,
        true,
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_glow_pink
    };
    theme.panel_styles[to_index(UiPanelThemeRole::List)] = UiPanelStyle{
        true,
        true,
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_mirage_blue
    };

    theme.window_style = UiWindowStyle{
        true,
        true,
        elysia::core::colors::elysia_deep_sea,
        elysia::core::colors::elysia_lilac
    };

    theme.chrome_container_style = UiChromeContainerStyle{
        true,
        true,
        true,
        elysia::core::colors::elysia_phantom_sea,
        elysia::core::colors::elysia_lilac,
        elysia::core::colors::elysia_mirage_blue
    };

    theme.button_styles[to_index(UiButtonThemeRole::Default)] = UiButtonStyle{
        interactive_chrome,
        text
    };
    theme.button_styles[to_index(UiButtonThemeRole::Primary)] = UiButtonStyle{
        make_chrome(
            make_surface_colors(
                elysia::core::colors::elysia_phantom_sea,
                elysia::core::colors::elysia_mirage_blue,
                elysia::core::colors::elysia_lilac,
                elysia::core::colors::gray_700),
            UiEnabledDisabledColors{ elysia::core::colors::elysia_lilac,elysia::core::colors::steel_blue }),
        text
    };
    theme.button_styles[to_index(UiButtonThemeRole::Danger)] = UiButtonStyle{
        make_chrome(
            make_surface_colors(
                elysia::core::colors::purple_700,
                elysia::core::colors::pink_700,
                elysia::core::colors::elysia_glow_pink,
                elysia::core::colors::gray_700),
            UiEnabledDisabledColors{ elysia::core::colors::elysia_glow_pink,elysia::core::colors::steel_blue }),
        text
    };

    theme.checkbox_style = UiCheckboxStyle{
        interactive_chrome,
        UiEnabledDisabledColors{ elysia::core::colors::elysia_glow_pink,elysia::core::colors::steel_blue },
        UiCheckboxMarkStyle::Checkmark
    };

    theme.radio_button_style = UiRadioButtonStyle{
        interactive_chrome,
        UiEnabledDisabledColors{ elysia::core::colors::elysia_glow_pink,elysia::core::colors::steel_blue },
        text
    };

    theme.drag_handle_style = UiDragHandleStyle{
        elysia::core::Vector2{ 18.0f,18.0f },
        std::nullopt,
        interactive_chrome
    };

    theme.slider_style = UiSliderStyle{
        UiChromeStyle{ interactive_surface,border,false,true },
        UiEnabledDisabledColors{ elysia::core::colors::elysia_glow_pink,elysia::core::colors::steel_blue },
        text,
        theme.drag_handle_style
    };

    theme.text_input_style = UiTextInputStyle{
        interactive_chrome,
        secondary_text,
        placeholder_text,
        elysia::core::colors::elysia_glow_pink
    };

    theme.scroll_container_style = UiScrollContainerStyle{
        UiScrollBarStyle{
            10.0f,
            4.0f,
            24.0f,
            true,
            elysia::core::colors::elysia_deep_sea,
            elysia::core::colors::elysia_phantom_sea,
            elysia::core::colors::elysia_mirage_blue,
            elysia::core::colors::gray_700,
            elysia::core::colors::elysia_lilac,
            elysia::core::colors::elysia_glow_pink,
            elysia::core::colors::elysia_rose_pink,
            elysia::core::colors::steel_blue
        },
        true,
        elysia::core::colors::elysia_phantom_sea,
        true,
        elysia::core::colors::elysia_lilac
    };

    return theme;
}
}

const UiPanelStyle& UiTheme::panel(UiPanelThemeRole role) const noexcept
{
    return panel_styles[to_index(role)];
}

const UiLabelStyle& UiTheme::label(UiLabelThemeRole role) const noexcept
{
    return label_styles[to_index(role)];
}

const UiButtonStyle& UiTheme::button(UiButtonThemeRole role) const noexcept
{
    return button_styles[to_index(role)];
}

const UiBarStyle& UiTheme::bar(UiBarThemeRole role) const noexcept
{
    return bar_styles[to_index(role)];
}

UiTheme make_builtin_theme(UiBuiltinTheme theme) noexcept
{
    switch (theme)
    {
    case UiBuiltinTheme::ElysiaLight:
        return make_elysia_light_theme();
    case UiBuiltinTheme::ElysiaDark:
        return make_elysia_dark_theme();
    case UiBuiltinTheme::BlueGlassMoon:
    default:
        return make_blue_glass_moon_theme();
    }
}

const UiTheme& builtin_theme(UiBuiltinTheme theme) noexcept
{
    static const UiTheme blue_glass_moon = make_blue_glass_moon_theme();
    static const UiTheme elysia_light = make_elysia_light_theme();
    static const UiTheme elysia_dark = make_elysia_dark_theme();

    switch (theme)
    {
    case UiBuiltinTheme::ElysiaLight:
        return elysia_light;
    case UiBuiltinTheme::ElysiaDark:
        return elysia_dark;
    case UiBuiltinTheme::BlueGlassMoon:
    default:
        return blue_glass_moon;
    }
}
}
