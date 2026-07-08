#pragma once

#include "ui_interaction_style.h"

namespace elysia::ui
{
struct UiLabelStyle
{
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

struct UiNumberStyle
{
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

struct UiBarStyle
{
    elysia::core::Color background{};
    elysia::core::Color fill{};
    elysia::core::Color border{};
    bool draw_border = false;
};

struct UiPanelStyle
{
    bool draw_background = false;
    bool draw_border = false;
    elysia::core::Color background{};
    elysia::core::Color border{};
};

struct UiPopupStyle
{
    bool draw_background = true;
    bool draw_border = true;
    elysia::core::Color background{};
    elysia::core::Color border{};
};

struct UiWindowStyle
{
    bool draw_background = false;
    bool draw_border = false;
    elysia::core::Color background{};
    elysia::core::Color border{};
};
}
