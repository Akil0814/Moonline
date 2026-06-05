#pragma once

#include "ui_theme_roles.h"
#include "ui_style.h"

struct UiTheme
{
    PanelStyle _default_panel;
    PanelStyle _screen_panel;
    PanelStyle _dialog_panel;
    PanelStyle _list_panel;

    LabelStyle _default_label;
    LabelStyle _title_label;
    LabelStyle _subtitle_label;
    LabelStyle _muted_label;

    ButtonStyle _default_button;
    ButtonStyle _primary_button;
    ButtonStyle _danger_button;

    BarStyle _default_bar;
    BarStyle _progress_bar;

    ScrollBarStyle _scroll_bar;
    SliderStyle _slider;
    ToggleStyle _toggle;
    TextInputStyle _text_input;
};

UiTheme make_loading_blue_theme();
