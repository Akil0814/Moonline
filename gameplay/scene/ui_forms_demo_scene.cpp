#include "ui_forms_demo_scene.h"

#include "main_menu_scene.h"

#include "../../engine/core/scene/scene_manager.h"
#include "../../engine/ui/style/ui_style.h"

void UiFormsDemoScene::on_enter()
{
    ensure_ui();
    reset();
}

void UiFormsDemoScene::on_exit()
{
}

void UiFormsDemoScene::on_update(double delta)
{
    Scene::on_update(delta);
}

void UiFormsDemoScene::on_render(SDL_Renderer* renderer)
{
    Scene::on_render(renderer);
}

void UiFormsDemoScene::on_input(
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

void UiFormsDemoScene::reset()
{
    clear_objects();
    ensure_ui();

    if (_screen)
    {
        _screen->reset();
        _screen->set_world_position({ 0.0f, 0.0f });
        _screen->set_size({ 1280.0f, 720.0f });
        _screen->set_direction(UiLayoutDirection::Vertical);
        _screen->set_anchor(UiLayoutAnchor::Center);
        _screen->set_cross_align(UiLayoutAlign::Center);
        _screen->set_spacing(20.0f);
        _screen->set_padding({ 92.0f, 84.0f, 92.0f, 84.0f });
        _screen->set_transition_enabled(true);
        _screen->set_panel_theme_role(UiPanelThemeRole::Screen);
    }

    if (_title_label)
    {
        _title_label->reset();
        _title_label->set_text("UI Forms Demo");
        _title_label->set_font_key("ui.default");
        _title_label->set_auto_size(true);
        _title_label->set_padding(8);
        _title_label->set_horizontal_align(TextHorizontalAlign::Center);
        _title_label->set_vertical_align(TextVerticalAlign::Center);

        _title_label->set_label_theme_role(UiLabelThemeRole::Title);
    }

    if (_subtitle_label)
    {
        _subtitle_label->reset();
        _subtitle_label->set_text("Edit fields and controls to validate the shared UI system.");
        _subtitle_label->set_font_key("ui.default");
        _subtitle_label->set_auto_size(true);
        _subtitle_label->set_padding(4);
        _subtitle_label->set_horizontal_align(TextHorizontalAlign::Center);
        _subtitle_label->set_vertical_align(TextVerticalAlign::Center);

        _subtitle_label->set_label_theme_role(UiLabelThemeRole::Subtitle);
    }

    if (_footer_label)
    {
        _footer_label->reset();
        _footer_label->set_text("Up / Down Focus  |  Left / Right Adjust  |  Type To Edit  |  Esc Back");
        _footer_label->set_font_key("ui.default");
        _footer_label->set_auto_size(true);
        _footer_label->set_padding(4);
        _footer_label->set_horizontal_align(TextHorizontalAlign::Center);
        _footer_label->set_vertical_align(TextVerticalAlign::Center);

        _footer_label->set_label_theme_role(UiLabelThemeRole::Muted);
    }

    if (_form_grid)
    {
        _form_grid->reset();
        _form_grid->set_size({ 820.0f, 300.0f });
        _form_grid->set_column_count(2);
        _form_grid->set_cell_spacing(20.0f, 16.0f);
        _form_grid->set_padding({ 24.0f, 24.0f, 24.0f, 24.0f });
        _form_grid->set_cell_align(UiLayoutAlign::Start, UiLayoutAlign::Center);
    }

    rebuild_form();

    if (_screen && _title_label && _subtitle_label && _form_grid && _back_button && _footer_label)
    {
        _screen->clear_focusable_controls();

        UiLayoutChildOptions centered_options;
        centered_options._use_custom_cross_align = true;
        centered_options._cross_align = UiLayoutAlign::Center;

        UiLayoutChildOptions grid_options = centered_options;
        grid_options._margin.top = 18.0f;
        grid_options._margin.bottom = 12.0f;

        _screen->add_child(_title_label, centered_options);
        _screen->add_child(_subtitle_label, centered_options);
        _screen->add_child(_form_grid, grid_options);
        _screen->add_child(_back_button, centered_options);
        _screen->add_child(_footer_label, centered_options);

        _screen->register_focusable_control(_name_input);
        _screen->register_focusable_control(_password_input);
        _screen->register_focusable_control(_toggle);
        _screen->register_focusable_control(_slider);
        _screen->register_focusable_control(_back_button);
        _screen->set_focused_control(0);
        _screen->open();

        add_object(_screen);
    }
}

void UiFormsDemoScene::ensure_ui()
{
    if (!_screen)
    {
        _screen = std::make_shared<UiScreen>();
    }

    if (!_title_label)
    {
        _title_label = std::make_shared<UiLabel>();
    }

    if (!_subtitle_label)
    {
        _subtitle_label = std::make_shared<UiLabel>();
    }

    if (!_footer_label)
    {
        _footer_label = std::make_shared<UiLabel>();
    }

    if (!_form_grid)
    {
        _form_grid = std::make_shared<UiGridLayout>();
    }

    if (!_name_label)
    {
        _name_label = std::make_shared<UiLabel>();
    }

    if (!_name_input)
    {
        _name_input = std::make_shared<UiTextInput>();
    }

    if (!_password_label)
    {
        _password_label = std::make_shared<UiLabel>();
    }

    if (!_password_input)
    {
        _password_input = std::make_shared<UiTextInput>();
    }

    if (!_toggle_label)
    {
        _toggle_label = std::make_shared<UiLabel>();
    }

    if (!_toggle)
    {
        _toggle = std::make_shared<UiToggle>();
    }

    if (!_slider_label)
    {
        _slider_label = std::make_shared<UiLabel>();
    }

    if (!_slider)
    {
        _slider = std::make_shared<UiSlider>();
    }

    if (!_back_button)
    {
        _back_button = std::make_shared<UiTextButton>(Vector2::zero(), Vector2{ 240.0f, 56.0f });
    }
}

void UiFormsDemoScene::rebuild_form()
{
    if (!_form_grid)
    {
        return;
    }

    _form_grid->clear_children();

    TextInputStyle input_style;
    input_style._padding = 10;

    ToggleStyle toggle_style;
    SliderStyle slider_style;
    slider_style._value_precision = 0;

    if (_name_label)
    {
        _name_label->reset();
        _name_label->set_text("Player Name");
        _name_label->set_font_key("ui.default");
        _name_label->set_auto_size(false);
        _name_label->set_size({ 240.0f, 44.0f });
        _name_label->set_label_theme_role(UiLabelThemeRole::Default);
        _name_label->set_horizontal_align(TextHorizontalAlign::Left);
        _name_label->set_vertical_align(TextVerticalAlign::Center);
        _form_grid->add_child(_name_label);
    }

    if (_name_input)
    {
        _name_input->reset();
        _name_input->set_size({ 340.0f, 44.0f });
        _name_input->set_font_key("ui.default");
        _name_input->set_placeholder_text("Type the player name");
        _name_input->set_max_length(24);
        _name_input->set_on_text_changed(
            [this](const std::string& text)
            {
                set_status_text("name -> " + text);
            }
        );
        UiStyle::apply_text_input(*_name_input, input_style);
        _form_grid->add_child(_name_input);
    }

    if (_password_label)
    {
        _password_label->reset();
        _password_label->set_text("Secret Code");
        _password_label->set_font_key("ui.default");
        _password_label->set_auto_size(false);
        _password_label->set_size({ 240.0f, 44.0f });
        _password_label->set_label_theme_role(UiLabelThemeRole::Default);
        _password_label->set_horizontal_align(TextHorizontalAlign::Left);
        _password_label->set_vertical_align(TextVerticalAlign::Center);
        _form_grid->add_child(_password_label);
    }

    if (_password_input)
    {
        _password_input->reset();
        _password_input->set_size({ 340.0f, 44.0f });
        _password_input->set_font_key("ui.default");
        _password_input->set_placeholder_text("Password mode");
        _password_input->set_password_mode(true);
        _password_input->set_max_length(16);
        _password_input->set_on_text_changed(
            [this](const std::string& text)
            {
                set_status_text("secret length -> " + std::to_string(text.size()));
            }
        );
        UiStyle::apply_text_input(*_password_input, input_style);
        _form_grid->add_child(_password_input);
    }

    if (_toggle_label)
    {
        _toggle_label->reset();
        _toggle_label->set_text("Controller Hints");
        _toggle_label->set_font_key("ui.default");
        _toggle_label->set_auto_size(false);
        _toggle_label->set_size({ 240.0f, 44.0f });
        _toggle_label->set_label_theme_role(UiLabelThemeRole::Default);
        _toggle_label->set_horizontal_align(TextHorizontalAlign::Left);
        _toggle_label->set_vertical_align(TextVerticalAlign::Center);
        _form_grid->add_child(_toggle_label);
    }

    if (_toggle)
    {
        _toggle->reset();
        _toggle->set_size({ 340.0f, 44.0f });
        _toggle->set_font_key("ui.default");
        _toggle->set_state_texts("Disabled", "Enabled");
        _toggle->set_value(true);
        _toggle->set_on_changed(
            [this](bool value)
            {
                set_status_text(value ? "controller hints -> enabled" : "controller hints -> disabled");
            }
        );
        UiStyle::apply_toggle(*_toggle, toggle_style);
        _form_grid->add_child(_toggle);
    }

    if (_slider_label)
    {
        _slider_label->reset();
        _slider_label->set_text("Menu Volume");
        _slider_label->set_font_key("ui.default");
        _slider_label->set_auto_size(false);
        _slider_label->set_size({ 240.0f, 44.0f });
        _slider_label->set_label_theme_role(UiLabelThemeRole::Default);
        _slider_label->set_horizontal_align(TextHorizontalAlign::Left);
        _slider_label->set_vertical_align(TextVerticalAlign::Center);
        _form_grid->add_child(_slider_label);
    }

    if (_slider)
    {
        _slider->reset();
        _slider->set_size({ 340.0f, 44.0f });
        _slider->set_font_key("ui.default");
        _slider->set_range(0.0f, 100.0f);
        _slider->set_value(70.0f);
        _slider->set_step(5.0f);
        _slider->set_value_suffix("%");
        _slider->set_show_value_text(true);
        _slider->set_on_value_changed(
            [this](float value)
            {
                set_status_text("menu volume -> " + std::to_string(static_cast<int>(value)) + "%");
            }
        );
        UiStyle::apply_slider(*_slider, slider_style);
        _form_grid->add_child(_slider);
    }

    if (_back_button)
    {
        _back_button->reset();
        _back_button->set_size({ 240.0f, 56.0f });
        _back_button->set_font_key("ui.default");
        _back_button->set_text("Back To Menu");
        _back_button->set_on_click(
            [this]()
            {
                return_to_main_menu();
            }
        );

        _back_button->set_button_theme_role(UiButtonThemeRole::Primary);
    }
}

void UiFormsDemoScene::return_to_main_menu()
{
    SceneManager::instance()->switch_to<MainMenuScene>();
}

void UiFormsDemoScene::set_status_text(const std::string& text)
{
    if (_subtitle_label)
    {
        _subtitle_label->set_text(text);
    }
}
