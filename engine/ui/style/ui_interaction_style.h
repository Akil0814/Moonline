#pragma once

#include "ui_palette.h"

namespace elysia::ui
{
// Paired colors used when a widget switches between enabled and disabled state.
struct UiEnabledDisabledColors
{
    elysia::core::Color enabled = UiPalette::text_primary;
    elysia::core::Color disabled = UiPalette::text_disabled;
};

// Interactive chrome colors keyed by idle, focus, pressed, and disabled state.
struct UiInteractiveColors
{
    elysia::core::Color idle = UiPalette::surface_interactive_idle;
    elysia::core::Color focused = UiPalette::surface_interactive_focused;
    elysia::core::Color active = UiPalette::surface_interactive_active;
    elysia::core::Color disabled = UiPalette::surface_disabled;
};

// Shared border/background styling used by interactive widgets.
struct UiChromeStyle
{
    UiInteractiveColors background{};
    UiEnabledDisabledColors border{
        UiPalette::border_default,
        UiPalette::border_disabled
    };
    bool draw_background = true;
    bool draw_border = true;
};

// Resolves the color that matches the widget's current interaction state.
[[nodiscard]] inline elysia::core::Color resolve_interactive_color(
    const UiInteractiveColors& colors,
    bool enabled,
    bool focused,
    bool active
) noexcept
{
    if (!enabled)
        return colors.disabled;
    if (active)
        return colors.active;
    if (focused)
        return colors.focused;
    return colors.idle;
}

// Resolves the color that matches the widget's enabled state.
[[nodiscard]] inline elysia::core::Color resolve_enabled_disabled_color(
    const UiEnabledDisabledColors& colors,
    bool enabled
) noexcept
{
    return enabled ? colors.enabled : colors.disabled;
}
}
