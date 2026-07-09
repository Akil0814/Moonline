#pragma once

#include "ui_palette.h"
#include "ui_visual_styles.h"
#include "../widgets/ui_button.h"
#include "../widgets/ui_checkbox.h"
#include "../widgets/ui_drag_handle.h"
#include "../widgets/ui_slider.h"
#include "../widgets/ui_text_input.h"
#include "../widgets/ui_labeled_checkbox.h"
#include "../containers/ui_scroll_container.h"

namespace elysia::ui
{
struct UiStyleDefaults
{
    [[nodiscard]] static UiEnabledDisabledColors text() noexcept
    {
        return UiEnabledDisabledColors{
            UiPalette::text_primary,
            UiPalette::text_disabled
        };
    }

    [[nodiscard]] static UiEnabledDisabledColors muted_text() noexcept
    {
        return UiEnabledDisabledColors{
            UiPalette::text_muted,
            UiPalette::text_disabled
        };
    }

    [[nodiscard]] static UiEnabledDisabledColors secondary_text() noexcept
    {
        return UiEnabledDisabledColors{
            UiPalette::text_secondary,
            UiPalette::text_disabled
        };
    }

    [[nodiscard]] static UiEnabledDisabledColors placeholder_text() noexcept
    {
        return UiEnabledDisabledColors{
            UiPalette::text_placeholder,
            UiPalette::text_disabled
        };
    }

    [[nodiscard]] static UiInteractiveColors interactive_surface() noexcept
    {
        return UiInteractiveColors{
            UiPalette::surface_interactive_idle,
            UiPalette::surface_interactive_focused,
            UiPalette::surface_interactive_active,
            UiPalette::surface_disabled
        };
    }

    [[nodiscard]] static UiChromeStyle interactive_chrome() noexcept
    {
        UiChromeStyle style;
        style.background = interactive_surface();
        style.border = UiEnabledDisabledColors{
            UiPalette::border_default,
            UiPalette::border_disabled
        };
        style.draw_background = true;
        style.draw_border = true;
        return style;
    }

    [[nodiscard]] static UiLabelStyle label() noexcept
    {
        UiLabelStyle style;
        style.text = UiPalette::text_primary;
        style.background = elysia::core::colors::transparent;
        style.draw_background = false;
        return style;
    }

    [[nodiscard]] static UiNumberStyle number() noexcept
    {
        UiNumberStyle style;
        style.text = UiPalette::text_primary;
        style.background = elysia::core::colors::transparent;
        style.draw_background = false;
        return style;
    }

    [[nodiscard]] static UiBarStyle bar() noexcept
    {
        UiBarStyle style;
        style.background = UiPalette::surface_elevated;
        style.fill = UiPalette::accent_fill;
        style.border = UiPalette::border_default;
        style.draw_border = false;
        return style;
    }

    [[nodiscard]] static UiPanelStyle panel() noexcept
    {
        UiPanelStyle style;
        style.draw_background = false;
        style.draw_border = false;
        style.background = UiPalette::surface_elevated;
        style.border = UiPalette::border_default;
        return style;
    }

    [[nodiscard]] static UiChromeContainerStyle chrome_container() noexcept
    {
        UiChromeContainerStyle style;
        style.draw_background = true;
        style.draw_border = true;
        style.draw_header_background = true;
        style.background = UiPalette::surface_elevated;
        style.border = UiPalette::border_default;
        style.header_background = UiPalette::surface_base;
        return style;
    }

    [[nodiscard]] static UiWindowStyle window() noexcept
    {
        UiWindowStyle style;
        style.draw_background = false;
        style.draw_border = false;
        style.background = UiPalette::surface_base;
        style.border = UiPalette::border_focus;
        return style;
    }

    [[nodiscard]] static UiButtonStyle button() noexcept
    {
        UiButtonStyle style;
        style.chrome = interactive_chrome();
        style.text = text();
        return style;
    }

    [[nodiscard]] static UiCheckboxStyle checkbox() noexcept
    {
        UiCheckboxStyle style;
        style.chrome = interactive_chrome();
        style.mark = UiEnabledDisabledColors{
            UiPalette::accent_fill,
            UiPalette::text_disabled
        };
        style.mark_style = UiCheckboxMarkStyle::Checkmark;
        return style;
    }

    [[nodiscard]] static UiDragHandleStyle drag_handle() noexcept
    {
        UiDragHandleStyle style;
        style.size = elysia::core::Vector2{ 18.0f,18.0f };
        style.chrome = interactive_chrome();
        return style;
    }

    [[nodiscard]] static UiSliderStyle slider() noexcept
    {
        UiSliderStyle style;
        style.chrome = UiChromeStyle{};
        style.chrome.draw_background = false;
        style.chrome.draw_border = false;
        style.fill = UiEnabledDisabledColors{
            UiPalette::accent_fill,
            UiPalette::text_disabled
        };
        style.text = text();
        style.handle = drag_handle();
        return style;
    }

    [[nodiscard]] static UiTextInputStyle text_input() noexcept
    {
        UiTextInputStyle style;
        style.chrome = interactive_chrome();
        style.text = secondary_text();
        style.placeholder = placeholder_text();
        style.caret = UiPalette::caret;
        return style;
    }

    [[nodiscard]] static UiScrollBarStyle scrollbar() noexcept
    {
        UiScrollBarStyle style;
        style.thickness = 10.0f;
        style.margin = 4.0f;
        style.min_thumb_length = 24.0f;
        style.draw_track = true;
        style.track_idle_color = UiPalette::scrollbar_track_idle;
        style.track_focused_color = UiPalette::scrollbar_track_focused;
        style.track_dragging_color = UiPalette::scrollbar_track_active;
        style.track_disabled_color = UiPalette::scrollbar_track_disabled;
        style.thumb_idle_color = UiPalette::scrollbar_thumb_idle;
        style.thumb_focused_color = UiPalette::scrollbar_thumb_focused;
        style.thumb_dragging_color = UiPalette::scrollbar_thumb_active;
        style.thumb_disabled_color = UiPalette::scrollbar_thumb_disabled;
        return style;
    }

    [[nodiscard]] static UiScrollContainerStyle scroll_container() noexcept
    {
        UiScrollContainerStyle style;
        style.scrollbar = scrollbar();
        style.draw_background = true;
        style.background_color = UiPalette::surface_elevated;
        style.draw_border = false;
        style.border_color = UiPalette::border_default;
        return style;
    }

    [[nodiscard]] static UiEnabledDisabledColors labeled_checkbox_text() noexcept
    {
        return text();
    }
};
}

