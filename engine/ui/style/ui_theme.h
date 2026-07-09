#pragma once

#include "ui_theme_roles.h"
#include "ui_visual_styles.h"
#include "../containers/ui_scroll_container.h"
#include "../widgets/ui_button.h"
#include "../widgets/ui_checkbox.h"
#include "../widgets/ui_drag_handle.h"
#include "../widgets/ui_radio_button.h"
#include "../widgets/ui_slider.h"
#include "../widgets/ui_text_input.h"

#include <array>
#include <cstddef>

namespace elysia::ui
{
// Identifies one built-in, fully-specified UI theme.
enum class UiBuiltinTheme
{
    BlueGlassMoon,
    ElysiaLight,
    ElysiaDark,
    EvangelionUnit00,
    EvangelionUnit01,
    EvangelionUnit02,
    QuietSlate
};

// Stores the complete default style set for one theme, including both
// role-based control variants and one-per-type fallbacks.
struct UiTheme
{
    static constexpr std::size_t panel_role_count = 4;
    static constexpr std::size_t label_role_count = 4;
    static constexpr std::size_t button_role_count = 3;
    static constexpr std::size_t bar_role_count = 2;

    std::array<UiPanelStyle,panel_role_count> panel_styles{};
    std::array<UiLabelStyle,label_role_count> label_styles{};
    std::array<UiButtonStyle,button_role_count> button_styles{};
    std::array<UiBarStyle,bar_role_count> bar_styles{};

    UiNumberStyle number_style{};
    UiWindowStyle window_style{};
    UiChromeContainerStyle chrome_container_style{};
    UiCheckboxStyle checkbox_style{};
    UiRadioButtonStyle radio_button_style{};
    UiDragHandleStyle drag_handle_style{};
    UiSliderStyle slider_style{};
    UiTextInputStyle text_input_style{};
    UiScrollContainerStyle scroll_container_style{};

    // Role-based accessors are only used by controls that opt into the
    // corresponding theme-role API. Other controls read the type-default fields.
    [[nodiscard]] const UiPanelStyle& panel(UiPanelThemeRole role = UiPanelThemeRole::Default) const noexcept;
    [[nodiscard]] const UiLabelStyle& label(UiLabelThemeRole role = UiLabelThemeRole::Default) const noexcept;
    [[nodiscard]] const UiButtonStyle& button(UiButtonThemeRole role = UiButtonThemeRole::Default) const noexcept;
    [[nodiscard]] const UiBarStyle& bar(UiBarThemeRole role = UiBarThemeRole::Default) const noexcept;
};

// Builds an owned theme snapshot for the requested built-in theme.
[[nodiscard]] UiTheme make_builtin_theme(UiBuiltinTheme theme) noexcept;
// Returns a cached built-in theme instance for callers that only need a shared view.
[[nodiscard]] const UiTheme& builtin_theme(UiBuiltinTheme theme) noexcept;
}
