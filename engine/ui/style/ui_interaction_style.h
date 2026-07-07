#pragma once

#include "../../core/render/color.h"

namespace elysia::ui
{
struct UiEnabledDisabledColors
{
    elysia::core::Color enabled{};
    elysia::core::Color disabled{};
};

struct UiInteractiveColors
{
    elysia::core::Color idle{};
    elysia::core::Color focused{};
    elysia::core::Color active{};
    elysia::core::Color disabled{};
};

struct UiChromeStyle
{
    UiInteractiveColors background{};
    UiEnabledDisabledColors border{};
    bool draw_background = false;
    bool draw_border = false;
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
