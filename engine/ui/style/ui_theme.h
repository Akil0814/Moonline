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
enum class UiBuiltinTheme
{
    ClassicBlue,
    Light,
    HighContrast
};

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

    [[nodiscard]] const UiPanelStyle& panel(UiPanelThemeRole role = UiPanelThemeRole::Default) const noexcept;
    [[nodiscard]] const UiLabelStyle& label(UiLabelThemeRole role = UiLabelThemeRole::Default) const noexcept;
    [[nodiscard]] const UiButtonStyle& button(UiButtonThemeRole role = UiButtonThemeRole::Default) const noexcept;
    [[nodiscard]] const UiBarStyle& bar(UiBarThemeRole role = UiBarThemeRole::Default) const noexcept;
};

[[nodiscard]] UiTheme make_builtin_theme(UiBuiltinTheme theme) noexcept;
[[nodiscard]] const UiTheme& builtin_theme(UiBuiltinTheme theme) noexcept;
}
