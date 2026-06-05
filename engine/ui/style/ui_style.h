#pragma once

/*#include "../ui_bar.h"
#include "../widgets/ui_button.h"
#include "../containers/ui_panel.h"
#include "../widgets/ui_label.h"
#include "../widgets/ui_text_input.h"
#include "../widgets/ui_slider.h"
#include "../widgets/ui_scroll_bar.h"
#include "../widgets/ui_toggle.h"*/

#include <SDL.h>

struct ButtonStyle
{
    SDL_Color _idle_color{ 180, 180, 180, 255 };
    SDL_Color _hovered_color{ 200, 200, 200, 255 };
    SDL_Color _pushed_color{ 130, 130, 130, 255 };
    SDL_Color _frame_color{ 0, 0, 0, 255 };

    SDL_Texture* _idle_texture = nullptr;
    SDL_Texture* _hovered_texture = nullptr;
    SDL_Texture* _pushed_texture = nullptr;

    bool _use_state_textures = false;
};

struct LabelStyle
{
    SDL_Color _text_color{ 255, 255, 255, 255 };
    SDL_Color _background_color{ 0, 0, 0, 0 };
    TextHorizontalAlign _horizontal_align = TextHorizontalAlign::Left;
    TextVerticalAlign _vertical_align = TextVerticalAlign::Top;
    bool _draw_background = false;
    int _padding = 0;
};

struct PanelStyle
{
    SDL_Color _background_color{ 24, 24, 32, 220 };
    SDL_Color _border_color{ 220, 220, 220, 255 };
    Uint8 _background_alpha = 255;
    bool _draw_background = true;
    bool _draw_border = false;
};

struct BarStyle
{
    SDL_Color _background_color{ 0, 43, 100, 255 };
    SDL_Color _fill_color{ 245, 255, 255, 255 };
    SDL_Color _border_color{ 0, 0, 0, 255 };
    BarFillDirection _fill_direction = BarFillDirection::LeftToRight;
    bool _draw_border = false;
    int _padding = 0;
};

struct ScrollBarStyle
{
    SDL_Color _track_color{ 32, 40, 56, 180 };
    SDL_Color _thumb_color{ 126, 156, 196, 255 };
    float _thickness = 8.0f;
    float _target_margin = 6.0f;
    float _min_thumb_size = 24.0f;
    bool _draw_track = true;
    bool _auto_hide = true;
};

struct SliderStyle
{
    BarStyle _bar_style;
    SDL_Color _thumb_color{ 232, 240, 250, 255 };
    SDL_Color _text_color{ 244, 244, 248, 255 };
    float _thumb_size = 16.0f;
    int _value_precision = 0;
};

struct ToggleStyle
{
    SDL_Color _text_color{ 244, 244, 248, 255 };
    SDL_Color _background_color{ 32, 42, 58, 220 };
    SDL_Color _focused_background_color{ 52, 78, 116, 235 };
    SDL_Color _border_color{ 112, 140, 180, 255 };
};

struct TextInputStyle
{
    SDL_Color _text_color{ 244, 244, 248, 255 };
    SDL_Color _placeholder_color{ 142, 154, 170, 255 };
    SDL_Color _background_color{ 28, 36, 52, 220 };
    SDL_Color _border_color{ 110, 140, 182, 255 };
    SDL_Color _focused_border_color{ 166, 208, 255, 255 };
    SDL_Color _caret_color{ 248, 248, 252, 255 };
    int _padding = 10;
};

class UiStyle
{
public:
/*    static void apply_button(UiButton& button, const ButtonStyle& style);
    static void apply_label(UiLabel& label, const LabelStyle& style);
    static void apply_panel(UiPanel& panel, const PanelStyle& style);
    static void apply_bar(UiBar& bar, const BarStyle& style);
    static void apply_scroll_bar(UiScrollBar& scroll_bar, const ScrollBarStyle& style);
    static void apply_slider(UiSlider& slider, const SliderStyle& style);
    static void apply_toggle(UiToggle& toggle, const ToggleStyle& style);
    static void apply_text_input(UiTextInput& text_input, const TextInputStyle& style);*/
};
