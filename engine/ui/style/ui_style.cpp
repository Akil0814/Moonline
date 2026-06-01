#include "ui_style.h"

void UiStyle::apply_button(UiButton& button, const ButtonStyle& style)
{
    if (style._use_state_textures)
    {
        button.set_state_textures(
            style._idle_texture,
            style._hovered_texture,
            style._pushed_texture
        );
        return;
    }

    button.clear_state_textures();
    button.set_state_colors(
        style._idle_color,
        style._hovered_color,
        style._pushed_color,
        style._frame_color
    );
}

void UiStyle::apply_label(UiLabel& label, const LabelStyle& style)
{
    label.set_text_color(style._text_color);
    label.set_background_color(style._background_color);
    label.set_draw_background(style._draw_background);
    label.set_horizontal_align(style._horizontal_align);
    label.set_vertical_align(style._vertical_align);
    label.set_padding(style._padding);
}

void UiStyle::apply_panel(UiPanel& panel, const PanelStyle& style)
{
    panel.set_background_color(style._background_color);
    panel.set_border_color(style._border_color);
    panel.set_background_alpha(style._background_alpha);
    panel.set_draw_background(style._draw_background);
    panel.set_draw_border(style._draw_border);
}

void UiStyle::apply_bar(Bar& bar, const BarStyle& style)
{
    bar.set_background_color(style._background_color);
    bar.set_fill_color(style._fill_color);
    bar.set_border_color(style._border_color);
    bar.set_fill_direction(style._fill_direction);
    bar.set_draw_border(style._draw_border);
    bar.set_padding(style._padding);
}

void UiStyle::apply_scroll_bar(UiScrollBar& scroll_bar, const ScrollBarStyle& style)
{
    scroll_bar.set_track_color(style._track_color);
    scroll_bar.set_thumb_color(style._thumb_color);
    scroll_bar.set_thickness(style._thickness);
    scroll_bar.set_target_margin(style._target_margin);
    scroll_bar.set_min_thumb_size(style._min_thumb_size);
    scroll_bar.set_draw_track(style._draw_track);
    scroll_bar.set_auto_hide(style._auto_hide);
}

void UiStyle::apply_slider(UiSlider& slider, const SliderStyle& style)
{
    apply_bar(slider.bar(), style._bar_style);
    slider.set_thumb_color(style._thumb_color);
    slider.set_text_color(style._text_color);
    slider.set_thumb_size(style._thumb_size);
    slider.set_value_precision(style._value_precision);
}

void UiStyle::apply_toggle(UiToggle& toggle, const ToggleStyle& style)
{
    toggle.set_text_color(style._text_color);
    toggle.set_background_color(style._background_color);
    toggle.set_focused_background_color(style._focused_background_color);
    toggle.set_border_color(style._border_color);
}

void UiStyle::apply_text_input(UiTextInput& text_input, const TextInputStyle& style)
{
    text_input.set_text_color(style._text_color);
    text_input.set_placeholder_color(style._placeholder_color);
    text_input.set_background_color(style._background_color);
    text_input.set_border_color(style._border_color);
    text_input.set_focused_border_color(style._focused_border_color);
    text_input.set_caret_color(style._caret_color);
    text_input.set_padding(style._padding);
}
