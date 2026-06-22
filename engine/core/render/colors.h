#pragma once

#include "color.h"

namespace elysia::core
{
namespace colors
{
inline constexpr Color transparent{ 0, 0, 0, 0 };
inline constexpr Color white{ 255, 255, 255, 255 };
inline constexpr Color black{ 0, 0, 0, 255 };

// Reusable palette tokens. Semantic UI roles should compose from these.
inline constexpr Color frosted_white{ 248, 252, 255, 255 };
inline constexpr Color alice_blue{ 240, 248, 255, 255 };
inline constexpr Color glacial_white{ 245, 255, 255, 255 };
inline constexpr Color powder_blue{ 172, 204, 232, 255 };
inline constexpr Color steel_blue{ 140, 166, 194, 255 };
inline constexpr Color sky_blue{ 118, 168, 214, 255 };
inline constexpr Color royal_blue{ 36, 76, 138, 255 };
inline constexpr Color cobalt_blue{ 20, 48, 100, 255 };
inline constexpr Color deep_cobalt_blue{ 0, 43, 100, 255 };
inline constexpr Color midnight_blue{ 12, 34, 78, 255 };
inline constexpr Color slate_blue{ 18, 28, 42, 224 };
inline constexpr Color abyss_blue{ 8, 16, 28, 255 };

inline constexpr Color ui_text_default = alice_blue;
inline constexpr Color ui_text_title = frosted_white;
inline constexpr Color ui_text_subtitle = powder_blue;
inline constexpr Color ui_text_muted = steel_blue;
}

}
