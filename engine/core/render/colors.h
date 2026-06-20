#pragma once

#include "color.h"

namespace colors
{
inline constexpr Color transparent{ 0, 0, 0, 0 };
inline constexpr Color white{ 255, 255, 255, 255 };
inline constexpr Color black{ 0, 0, 0, 255 };

inline constexpr Color ui_text_default{ 240, 248, 255, 255 };
inline constexpr Color ui_text_title{ 248, 252, 255, 255 };
inline constexpr Color ui_text_subtitle{ 172, 204, 232, 255 };
inline constexpr Color ui_text_muted{ 140, 166, 194, 255 };

inline constexpr Color loading_blue_button_idle{ 20, 48, 100, 255 };
inline constexpr Color loading_blue_button_hovered{ 36, 76, 138, 255 };
inline constexpr Color loading_blue_button_pushed{ 12, 34, 78, 255 };
inline constexpr Color loading_blue_button_frame{ 118, 168, 214, 255 };

inline constexpr Color loading_blue_panel_default{ 18, 28, 42, 224 };
inline constexpr Color loading_blue_panel_screen{ 8, 16, 28, 255 };

inline constexpr Color loading_blue_bar_background{ 0, 43, 100, 255 };
inline constexpr Color loading_blue_bar_fill{ 245, 255, 255, 255 };
}
