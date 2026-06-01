#include "options_scene.h"

#include "main_menu_scene.h"

#include "../../engine/core/scene/scene_manager.h"
#include "../../engine/ui/style/ui_style.h"

void OptionsScene::on_enter()
{
    ensure_ui();
    reset();
}

void OptionsScene::on_exit()
{
}

void OptionsScene::on_update(double delta)
{
    Scene::on_update(delta);
}

void OptionsScene::on_render(SDL_Renderer* renderer)
{
    Scene::on_render(renderer);
}

void OptionsScene::on_input(
    const InputSnapshot& input,
    const std::vector<InputEvent>& events
)
{
    Scene::on_input(input, events);

    for (const InputEvent& event : events)
    {
        if (event.type == InputEventType::Pressed && event.action == InputAction::Cancel)
        {
            return_to_main_menu();
            return;
        }
    }
}

void OptionsScene::reset()
{
    clear_objects();
    ensure_ui();

    if (_screen)
    {
        _screen->reset();
        _screen->set_world_position({ 0.0f, 0.0f });
        _screen->set_size({ 1280.0f, 720.0f });
        _screen->set_direction(LayoutDirection::Vertical);
        _screen->set_anchor(LayoutAnchor::Center);
        _screen->set_cross_align(LayoutAlign::Center);
        _screen->set_spacing(16.0f);
        _screen->set_padding({ 110.0f, 88.0f, 110.0f, 88.0f });
        _screen->set_transition_enabled(true);

        PanelStyle screen_style;
        screen_style._background_color = SDL_Color{ 10, 16, 26, 255 };
        screen_style._draw_background = true;
        UiStyle::apply_panel(*_screen, screen_style);
    }

    if (_title_label)
    {
        _title_label->reset();
        _title_label->set_text("Options");
        _title_label->set_font_key("ui.default");
        _title_label->set_padding(8);
        _title_label->set_auto_size(true);

        LabelStyle title_style;
        title_style._text_color = SDL_Color{ 244, 244, 248, 255 };
        title_style._horizontal_align = TextHorizontalAlign::Center;
        title_style._vertical_align = TextVerticalAlign::Center;
        UiStyle::apply_label(*_title_label, title_style);
    }

    if (_subtitle_label)
    {
        _subtitle_label->reset();
        _subtitle_label->set_text("Adjust prototype settings with Left / Right.");
        _subtitle_label->set_font_key("ui.default");
        _subtitle_label->set_padding(4);
        _subtitle_label->set_auto_size(true);

        LabelStyle subtitle_style;
        subtitle_style._text_color = SDL_Color{ 166, 188, 220, 255 };
        subtitle_style._horizontal_align = TextHorizontalAlign::Center;
        subtitle_style._vertical_align = TextVerticalAlign::Center;
        UiStyle::apply_label(*_subtitle_label, subtitle_style);
    }

    if (_option_list)
    {
        _option_list->reset();
        _option_list->set_size({ 620.0f, 360.0f });

        UiOptionListStyle list_style;
        list_style._row_size = { 540.0f, 62.0f };
        list_style._row_spacing = 12.0f;
        list_style._panel_padding = 14.0f;
        list_style._row_background_color = SDL_Color{ 28, 36, 52, 220 };
        list_style._row_selected_background_color = SDL_Color{ 52, 78, 116, 235 };
        list_style._row_disabled_background_color = SDL_Color{ 22, 26, 34, 180 };
        list_style._row_border_color = SDL_Color{ 110, 140, 182, 255 };
        list_style._label_text_color = SDL_Color{ 246, 246, 248, 255 };
        list_style._disabled_text_color = SDL_Color{ 130, 140, 152, 255 };
        list_style._toggle_style._text_color = SDL_Color{ 194, 218, 248, 255 };
        list_style._slider_style._text_color = SDL_Color{ 194, 218, 248, 255 };
        list_style._slider_style._bar_style._background_color = SDL_Color{ 18, 24, 36, 255 };
        list_style._slider_style._bar_style._fill_color = SDL_Color{ 120, 154, 206, 255 };
        list_style._slider_style._bar_style._border_color = SDL_Color{ 110, 140, 182, 255 };
        list_style._slider_style._bar_style._draw_border = true;
        _option_list->set_style(list_style);
        _option_list->set_font_key("ui.default");
        _option_list->set_on_value_changed(
            [this](int row_index, const std::string& id, const std::string& value)
            {
                on_option_value_changed(row_index, id, value);
            }
        );
        rebuild_options();
    }

    if (_scroll_bar)
    {
        _scroll_bar->reset();
        _scroll_bar->set_target(_option_list.get());

        ScrollBarStyle scroll_bar_style;
        scroll_bar_style._track_color = SDL_Color{ 18, 24, 36, 180 };
        scroll_bar_style._thumb_color = SDL_Color{ 120, 154, 206, 255 };
        scroll_bar_style._thickness = 10.0f;
        scroll_bar_style._target_margin = 8.0f;
        scroll_bar_style._min_thumb_size = 28.0f;
        scroll_bar_style._draw_track = true;
        scroll_bar_style._auto_hide = true;
        UiStyle::apply_scroll_bar(*_scroll_bar, scroll_bar_style);
    }

    if (_footer_label)
    {
        _footer_label->reset();
        _footer_label->set_text("Arrow Keys Navigate  |  Left / Right Change  |  Esc Back");
        _footer_label->set_font_key("ui.default");
        _footer_label->set_padding(4);
        _footer_label->set_auto_size(true);

        LabelStyle footer_style;
        footer_style._text_color = SDL_Color{ 148, 164, 186, 255 };
        footer_style._horizontal_align = TextHorizontalAlign::Center;
        footer_style._vertical_align = TextVerticalAlign::Center;
        UiStyle::apply_label(*_footer_label, footer_style);
    }

    if (_screen && _title_label && _subtitle_label && _option_list && _footer_label)
    {
        _screen->clear_focusable_controls();

        LayoutChildOptions centered_options;
        centered_options._use_custom_cross_align = true;
        centered_options._cross_align = LayoutAlign::Center;

        LayoutChildOptions list_options = centered_options;
        list_options._margin.top = 16.0f;
        list_options._margin.bottom = 12.0f;

        LayoutChildOptions footer_options = centered_options;
        footer_options._margin.top = 8.0f;

        _screen->add_child(_title_label, centered_options);
        _screen->add_child(_subtitle_label, centered_options);
        _screen->add_child(_option_list, list_options);
        _screen->add_child(_footer_label, footer_options);
        _screen->register_focusable_control(_option_list);
        _screen->set_focused_control(0);
        _screen->open();
        add_object(_screen);

        if (_scroll_bar)
        {
            add_object(_scroll_bar);
        }
    }
}

void OptionsScene::ensure_ui()
{
    if (!_screen)
    {
        _screen = std::make_shared<UiScreen>();
    }

    if (!_title_label)
    {
        _title_label = std::make_shared<Label>();
    }

    if (!_subtitle_label)
    {
        _subtitle_label = std::make_shared<Label>();
    }

    if (!_footer_label)
    {
        _footer_label = std::make_shared<Label>();
    }

    if (!_option_list)
    {
        _option_list = std::make_shared<UiOptionList>();
    }

    if (!_scroll_bar)
    {
        _scroll_bar = std::make_shared<UiScrollBar>();
    }
}

void OptionsScene::rebuild_options()
{
    if (!_option_list)
    {
        return;
    }

    std::vector<UiOptionListItem> items{
        { "display_mode", "Display Mode", UiOptionControlType::Toggle, { false, "Fullscreen", "Windowed" }, {}, true },
        { "vsync", "VSync", UiOptionControlType::Toggle, { true, "Off", "On" }, {}, true },
        { "bgm_volume", "BGM Volume", UiOptionControlType::Slider, {}, { 0.0f, 100.0f, 75.0f, 5.0f, 0, "%" }, true },
        { "sfx_volume", "SFX Volume", UiOptionControlType::Slider, {}, { 0.0f, 100.0f, 75.0f, 5.0f, 0, "%" }, true },
        { "language", "Language", UiOptionControlType::Toggle, { false, "English", "Chinese" }, {}, true },
        { "hit_effects", "Hit Effects", UiOptionControlType::Toggle, { true, "Disabled", "Enabled" }, {}, true },
        { "screen_shake", "Screen Shake", UiOptionControlType::Toggle, { true, "Disabled", "Enabled" }, {}, true }
    };

    _option_list->set_items(items);
    _option_list->set_selected_row_index(0);
}

void OptionsScene::on_option_value_changed(
    int row_index,
    const std::string& id,
    const std::string& value
)
{
    (void)row_index;

    if (!_subtitle_label)
    {
        return;
    }

    _subtitle_label->set_text(id + " -> " + value);
}

void OptionsScene::return_to_main_menu()
{
    SceneManager::instance()->switch_to<MainMenuScene>();
}
