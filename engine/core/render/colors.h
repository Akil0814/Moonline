#pragma once

#include "color.h"

namespace elysia::core
{
namespace colors
{
inline constexpr Color transparent{ 0, 0, 0, 0 };       // #00000000
inline constexpr Color white{ 255, 255, 255, 255 };     // #FFFFFF
inline constexpr Color black{ 0, 0, 0, 255 };           // #000000

// Common neutral tokens.
inline constexpr Color gray_100{ 245, 245, 245, 255 };  // #F5F5F5
inline constexpr Color gray_300{ 224, 224, 224, 255 };  // #E0E0E0
inline constexpr Color gray_500{ 158, 158, 158, 255 };  // #9E9E9E
inline constexpr Color gray_700{ 97, 97, 97, 255 };     // #616161
inline constexpr Color gray_900{ 33, 33, 33, 255 };     // #212121

// Common accent families. Simple names map to the 500 tone.
inline constexpr Color red_300{ 229, 115, 115, 255 };      // #E57373
inline constexpr Color red_500{ 244, 67, 54, 255 };        // #F44336
inline constexpr Color red_700{ 211, 47, 47, 255 };        // #D32F2F
inline constexpr Color red = red_500;                      // #F44336

inline constexpr Color orange_300{ 255, 183, 77, 255 };   // #FFB74D
inline constexpr Color orange_500{ 255, 152, 0, 255 };    // #FF9800
inline constexpr Color orange_700{ 245, 124, 0, 255 };    // #F57C00
inline constexpr Color orange = orange_500;               // #FF9800

inline constexpr Color yellow_300{ 255, 241, 118, 255 };  // #FFF176
inline constexpr Color yellow_500{ 255, 235, 59, 255 };   // #FFEB3B
inline constexpr Color yellow_700{ 251, 192, 45, 255 };   // #FBC02D
inline constexpr Color yellow = yellow_500;               // #FFEB3B

inline constexpr Color green_300{ 129, 199, 132, 255 };   // #81C784
inline constexpr Color green_500{ 76, 175, 80, 255 };     // #4CAF50
inline constexpr Color green_700{ 56, 142, 60, 255 };     // #388E3C
inline constexpr Color green = green_500;                 // #4CAF50

inline constexpr Color cyan_300{ 77, 208, 225, 255 };     // #4DD0E1
inline constexpr Color cyan_500{ 0, 188, 212, 255 };      // #00BCD4
inline constexpr Color cyan_700{ 0, 151, 167, 255 };      // #0097A7
inline constexpr Color cyan = cyan_500;                   // #00BCD4

inline constexpr Color blue_300{ 100, 181, 246, 255 };    // #64B5F6
inline constexpr Color blue_500{ 33, 150, 243, 255 };     // #2196F3
inline constexpr Color blue_700{ 25, 118, 210, 255 };     // #1976D2
inline constexpr Color blue = blue_500;                   // #2196F3

inline constexpr Color purple_300{ 186, 104, 200, 255 };  // #BA68C8
inline constexpr Color purple_500{ 156, 39, 176, 255 };   // #9C27B0
inline constexpr Color purple_700{ 123, 31, 162, 255 };   // #7B1FA2
inline constexpr Color purple = purple_500;               // #9C27B0

inline constexpr Color pink_300{ 240, 98, 146, 255 };     // #F06292
inline constexpr Color pink_500{ 233, 30, 99, 255 };      // #E91E63
inline constexpr Color pink_700{ 194, 24, 91, 255 };      // #C2185B
inline constexpr Color pink = pink_500;                   // #E91E63

inline constexpr Color brown_300{ 161, 136, 127, 255 };   // #A1887F
inline constexpr Color brown_500{ 121, 85, 72, 255 };     // #795548
inline constexpr Color brown_700{ 93, 64, 55, 255 };      // #5D4037
inline constexpr Color brown = brown_500;                 // #795548

// Reusable palette tokens. Semantic UI roles should compose from these.
inline constexpr Color frosted_white{ 248, 252, 255, 255 }; // #F8FCFF
inline constexpr Color alice_blue{ 240, 248, 255, 255 };    // #F0F8FF
inline constexpr Color glacial_white{ 245, 255, 255, 255 }; // #F5FFFF
inline constexpr Color powder_blue{ 172, 204, 232, 255 };   // #ACCCE8
inline constexpr Color steel_blue{ 140, 166, 194, 255 };    // #8CA6C2
inline constexpr Color sky_blue{ 118, 168, 214, 255 };      // #76A8D6
inline constexpr Color royal_blue{ 36, 76, 138, 255 };      // #244C8A
inline constexpr Color cobalt_blue{ 20, 48, 100, 255 };     // #143064
inline constexpr Color deep_cobalt_blue{ 0, 43, 100, 255 }; // #002B64
inline constexpr Color midnight_blue{ 12, 34, 78, 255 };    // #0C224E
inline constexpr Color slate_blue{ 18, 28, 42, 224 };       // #121C2AE0
inline constexpr Color abyss_blue{ 8, 16, 28, 255 };        // #08101C

inline constexpr Color ui_text_default = alice_blue;        // #F0F8FF
inline constexpr Color ui_text_title = frosted_white;       // #F8FCFF
inline constexpr Color ui_text_subtitle = powder_blue;      // #ACCCE8
inline constexpr Color ui_text_muted = steel_blue;          // #8CA6C2

inline constexpr Color ui_state_success = green_500;        // #4CAF50
inline constexpr Color ui_state_warning = orange_500;       // #FF9800
inline constexpr Color ui_state_danger = red_500;           // #F44336
inline constexpr Color ui_state_info = blue_500;            // #2196F3
}

}
