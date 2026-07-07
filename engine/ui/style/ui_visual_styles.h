#pragma once

#include "ui_interaction_style.h"

namespace elysia::ui
{
struct UiLabelStyle
{
    elysia::core::Color text = UiPalette::text_primary;
    elysia::core::Color background = elysia::core::colors::transparent;
    bool draw_background = false;
};

struct UiNumberStyle
{
    elysia::core::Color text = UiPalette::text_primary;
    elysia::core::Color background = elysia::core::colors::transparent;
    bool draw_background = false;
};

struct UiBarStyle
{
    elysia::core::Color background = UiPalette::surface_elevated;
    elysia::core::Color fill = UiPalette::accent_fill;
    elysia::core::Color border = UiPalette::border_default;
    bool draw_border = false;
};

struct UiPanelStyle
{
    bool draw_background = false;
    bool draw_border = false;
    elysia::core::Color background = UiPalette::surface_elevated;
    elysia::core::Color border = UiPalette::border_default;
};

struct UiWindowStyle
{
    bool draw_background = false;
    bool draw_border = false;
    elysia::core::Color background = UiPalette::surface_base;
    elysia::core::Color border = UiPalette::border_focus;
};
}
