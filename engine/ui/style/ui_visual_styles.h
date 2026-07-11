#pragma once

#include "ui_interaction_style.h"
#include "../window/ui_overlay.h"

namespace elysia::ui
{
// Visual settings for text-only label widgets.
struct UiLabelStyle
{
    float corner_radius = 0.0f;
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

// Visual settings for wrapped long-form text blocks.
struct UiTextBlockStyle
{
    float corner_radius = 0.0f;
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

// Layout settings for a transient dropdown menu. Child colors come from existing themes.
struct UiDropdownStyle
{
    float popup_gap = 4.0f;
    float option_height = 42.0f;
    float popup_max_height = 240.0f;
};

// Visual settings for numeric glyph widgets.
struct UiNumberStyle
{
    float corner_radius = 0.0f;
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

// Visual settings for value bars and progress-like fills.
struct UiBarStyle
{
    float corner_radius = 0.0f;
    elysia::core::Color background{};
    elysia::core::Color fill{};
    elysia::core::Color border{};
    bool draw_border = true;
};

// Visual settings for simple panel containers.
struct UiPanelStyle
{
    float corner_radius = 0.0f;
    bool draw_background = true;
    bool draw_border = true;
    elysia::core::Color background{};
    elysia::core::Color border{};
};

// Visual settings for containers with a framed body and optional header chrome.
struct UiChromeContainerStyle
{
    float corner_radius = 0.0f;
    bool draw_background = true;
    bool draw_border = true;
    bool draw_header_background = true;
    elysia::core::Color background{};
    elysia::core::Color border{};
    elysia::core::Color header_background{};
};

// Visual settings for top-level UI windows.
struct UiWindowStyle
{
    float corner_radius = 0.0f;
    bool draw_background = true;
    bool draw_border = true;
    elysia::core::Color background{};
    elysia::core::Color border{};
};

// Layout defaults for composite reading dialogs.
struct UiDialogStyle
{
    float corner_radius = 0.0f;
    UiOverlayOptions overlay_defaults{};
    float close_button_height = 42.0f;
    float body_footer_spacing = 12.0f;
    int body_padding = 18;
    int text_padding = 10;
};
}
