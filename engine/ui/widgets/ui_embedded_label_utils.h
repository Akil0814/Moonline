#pragma once

#include "ui_label.h"

#include <string>

inline void ui_reset_embedded_label(
    UiLabel& label,
    const std::string& font_key,
    SDL_Color text_color,
    TextHorizontalAlign horizontal_align,
    TextVerticalAlign vertical_align
)
{
    label.reset();
    label.set_font_key(font_key);
    label.set_auto_size(false);
    label.set_horizontal_align(horizontal_align);
    label.set_vertical_align(vertical_align);
    label.set_text_color(text_color);
}

inline void ui_sync_embedded_label(
    UiLabel& label,
    const std::string& font_key,
    SDL_Color text_color,
    const Vector2& world_position,
    const Vector2& label_size,
    const std::string& text
)
{
    label.set_font_key(font_key);
    label.set_text_color(text_color);
    label.set_world_position(world_position);
    label.set_size(label_size);
    label.set_text(text);
}
