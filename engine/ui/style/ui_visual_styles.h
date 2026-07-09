#pragma once

#include "ui_interaction_style.h"
#include "../window/ui_overlay.h"

namespace elysia::ui
{
// Visual settings for text-only label widgets.
struct UiLabelStyle
{
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

// Visual settings for wrapped long-form text blocks.
struct UiTextBlockStyle
{
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

// Visual settings for numeric glyph widgets.
struct UiNumberStyle
{
    elysia::core::Color text{};
    elysia::core::Color background{};
    bool draw_background = false;
};

// Visual settings for value bars and progress-like fills.
struct UiBarStyle
{
    elysia::core::Color background{};
    elysia::core::Color fill{};
    elysia::core::Color border{};
    bool draw_border = true;
};

// Visual settings for simple panel containers.
struct UiPanelStyle
{
    bool draw_background = true;
    bool draw_border = true;
    elysia::core::Color background{};
    elysia::core::Color border{};
};

// Visual settings for containers with a framed body and optional header chrome.
struct UiChromeContainerStyle
{
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
    bool draw_background = true;
    bool draw_border = true;
    elysia::core::Color background{};
    elysia::core::Color border{};
};

// Layout defaults for composite reading dialogs.
struct UiDialogStyle
{
    UiOverlayOptions overlay_defaults{};
    float close_button_height = 42.0f;
    float body_footer_spacing = 12.0f;
    int body_padding = 18;
    int text_padding = 10;
    int title_point_size = 26;
    int body_point_size = 20;
};
}
