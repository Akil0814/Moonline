#pragma once

#include "ui_palette.h"

namespace elysia::ui
{
struct UiEnabledDisabledColors
{
    elysia::core::Color enabled = UiPalette::text_primary;
    elysia::core::Color disabled = UiPalette::text_disabled;
};

struct UiInteractiveColors
{
    elysia::core::Color idle = UiPalette::surface_interactive_idle;
    elysia::core::Color focused = UiPalette::surface_interactive_focused;
    elysia::core::Color active = UiPalette::surface_interactive_active;
    elysia::core::Color disabled = UiPalette::surface_disabled;
};

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

[[nodiscard]] inline elysia::core::Color resolve_enabled_disabled_color(
    const UiEnabledDisabledColors& colors,
    bool enabled
) noexcept
{
    return enabled ? colors.enabled : colors.disabled;
}
}