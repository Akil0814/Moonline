#pragma once

#include "../../core/render/colors.h"

namespace elysia::ui
{
struct UiEnabledDisabledColors
{
    elysia::core::Color enabled = elysia::core::colors::white;
    elysia::core::Color disabled = elysia::core::colors::gray_300;
};

struct UiInteractiveColors
{
    elysia::core::Color idle = elysia::core::colors::cobalt_blue;
    elysia::core::Color focused = elysia::core::colors::royal_blue;
    elysia::core::Color active = elysia::core::colors::midnight_blue;
    elysia::core::Color disabled = elysia::core::colors::gray_700;
};

struct UiChromeStyle
{
    UiInteractiveColors background{};
    UiEnabledDisabledColors border{
        elysia::core::colors::sky_blue,
        elysia::core::colors::gray_500
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
