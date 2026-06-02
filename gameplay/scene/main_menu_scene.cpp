#include "main_menu_scene.h"

#include "options_scene.h"
#include "ui_forms_demo_scene.h"

#include "../../engine/core/scene/scene_manager.h"
#include "../../engine/ui/style/ui_style.h"

#include <SDL.h>

void MainMenuScene::on_enter()
{
    ensure_ui();
    reset();
}

void MainMenuScene::on_exit()
{
    clear_objects();

    if (_menu_scroll_bar)
    {
        _menu_scroll_bar->set_target(nullptr);
    }

    if (_menu_list)
    {
        _menu_list->set_on_selection_changed({});
    }

    if (_quit_dialog)
    {
        _quit_dialog->set_on_action({});
    }

    _quit_dialog.reset();
    _menu_scroll_bar.reset();
    _menu_list.reset();
    _footer_label.reset();
    _subtitle_label.reset();
    _title_label.reset();
    _screen.reset();
}

void MainMenuScene::on_update(double delta)
{
    Scene::on_update(delta);
}

void MainMenuScene::on_render(SDL_Renderer* renderer)
{
    Scene::on_render(renderer);
}

void MainMenuScene::on_input(
    const InputSnapshot& input,
    const std::vector<InputEvent>& events
)
{
    Scene::on_input(input, events);
}

void MainMenuScene::reset()
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
        _screen->set_spacing(18.0f);
        _screen->set_padding({ 120.0f, 96.0f, 120.0f, 96.0f });
        _screen->set_transition_enabled(true);
        _screen->set_panel_theme_role(UiPanelThemeRole::Screen);
    }

    if (_title_label)
    {
        _title_label->reset();
        _title_label->set_text("Moonline");
        _title_label->set_font_key("ui.default");
        _title_label->set_padding(8);
        _title_label->set_auto_size(true);
        _title_label->set_horizontal_align(TextHorizontalAlign::Center);
        _title_label->set_vertical_align(TextVerticalAlign::Center);

        _title_label->set_label_theme_role(UiLabelThemeRole::Title);
    }

    if (_subtitle_label)
    {
        _subtitle_label->reset();
        _subtitle_label->set_text("Prototype Main Menu");
        _subtitle_label->set_font_key("ui.default");
        _subtitle_label->set_padding(4);
        _subtitle_label->set_auto_size(true);
        _subtitle_label->set_horizontal_align(TextHorizontalAlign::Center);
        _subtitle_label->set_vertical_align(TextVerticalAlign::Center);

        _subtitle_label->set_label_theme_role(UiLabelThemeRole::Subtitle);
    }

    if (_menu_list)
    {
        _menu_list->reset();
        _menu_list->set_size({ 420.0f, 280.0f });
        _menu_list->set_anchor(UiLayoutAnchor::TopCenter);
        _menu_list->set_cross_align(UiLayoutAlign::Center);
        _menu_list->set_spacing(14.0f);
        _menu_list->set_padding({ 12.0f, 12.0f, 12.0f, 12.0f });
        _menu_list->set_item_size({ 360.0f, 60.0f });
        _menu_list->set_font_key("ui.default");
        _menu_list->set_panel_theme_role(UiPanelThemeRole::List);
        _menu_list->set_on_selection_changed(
            [this](int index, const std::string& id, const std::string& text)
            {
                select_menu_item(index, id, text);
            }
        );
        rebuild_menu_items();
    }

    if (_menu_scroll_bar)
    {
        _menu_scroll_bar->reset();
        _menu_scroll_bar->set_target(_menu_list.get());
    }

    if (_quit_dialog)
    {
        _quit_dialog->reset();
        _quit_dialog->set_world_position({ 380.0f, 200.0f });
        _quit_dialog->set_title("Quit Moonline");
        _quit_dialog->set_message("Are you sure you want to quit this prototype build?");
        _quit_dialog->set_actions({
            { "cancel", "Cancel" },
            { "quit", "Quit" }
        });
        _quit_dialog->set_on_action(
            [this](const std::string& id, const std::string& text)
            {
                handle_quit_dialog_action(id, text);
            }
        );
    }

    if (_footer_label)
    {
        _footer_label->reset();
        _footer_label->set_text("Enter Confirm  |  Esc Cancel  |  Arrow Keys Navigate");
        _footer_label->set_font_key("ui.default");
        _footer_label->set_padding(4);
        _footer_label->set_auto_size(true);
        _footer_label->set_horizontal_align(TextHorizontalAlign::Center);
        _footer_label->set_vertical_align(TextVerticalAlign::Center);

        _footer_label->set_label_theme_role(UiLabelThemeRole::Muted);
    }

    if (_screen && _title_label && _subtitle_label && _menu_list && _footer_label)
    {
        _screen->clear_focusable_controls();

        UiLayoutChildOptions title_options;
        title_options._use_custom_cross_align = true;
        title_options._cross_align = UiLayoutAlign::Center;

        UiLayoutChildOptions subtitle_options = title_options;

        UiLayoutChildOptions menu_options = title_options;
        menu_options._margin.top = 16.0f;
        menu_options._margin.bottom = 12.0f;

        UiLayoutChildOptions footer_options = title_options;
        footer_options._margin.top = 10.0f;

        _screen->add_child(_title_label, title_options);
        _screen->add_child(_subtitle_label, subtitle_options);
        _screen->add_child(_menu_list, menu_options);
        _screen->add_child(_footer_label, footer_options);
        _screen->register_focusable_control(_menu_list);
        _screen->set_focused_control(0);
        _screen->open();
        add_object(_screen);
        if (_menu_scroll_bar)
        {
            add_object(_menu_scroll_bar);
        }
        if (_quit_dialog)
        {
            add_object(_quit_dialog);
        }
    }
}

void MainMenuScene::ensure_ui()
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

    if (!_menu_list)
    {
        _menu_list = std::make_shared<UiMenuList>();
    }

    if (!_menu_scroll_bar)
    {
        _menu_scroll_bar = std::make_shared<UiScrollBar>();
    }

    if (!_quit_dialog)
    {
        _quit_dialog = std::make_shared<UiDialog>(Vector2::zero(), Vector2::zero(), 10);
    }
}

void MainMenuScene::rebuild_menu_items()
{
    if (!_menu_list)
    {
        return;
    }

    std::vector<UiMenuListItem> items{
        { "start", "Start", true },
        { "options", "Options", true },
        { "gallery", "UI Demo", true },
        { "quit", "Quit", true }
    };

    _menu_list->set_items(items);
    _menu_list->set_selected_index(0);
}

void MainMenuScene::select_menu_item(
    int index,
    const std::string& id,
    const std::string& text
)
{
    (void)index;
    (void)text;

    if (!_subtitle_label)
    {
        return;
    }

    if (id == "quit")
    {
        open_quit_dialog();
        return;
    }

    if (id == "start")
    {
        _subtitle_label->set_text("Start selected. Gameplay scene is not wired yet.");
        return;
    }

    if (id == "options")
    {
        SceneManager::instance()->switch_to<OptionsScene>();
        return;
    }

    if (id == "gallery")
    {
        SceneManager::instance()->switch_to<UiFormsDemoScene>();
        return;
    }
}

void MainMenuScene::request_quit()
{
    SDL_Event quit_event{};
    quit_event.type = SDL_QUIT;
    SDL_PushEvent(&quit_event);
}

void MainMenuScene::open_quit_dialog()
{
    if (_subtitle_label)
    {
        _subtitle_label->set_text("Quit selected. Confirmation required.");
    }

    if (_screen)
    {
        _screen->set_input_enabled(false);
    }

    if (_menu_list)
    {
        _menu_list->set_active(false);
    }

    if (_quit_dialog)
    {
        _quit_dialog->show_dialog();
    }
}

void MainMenuScene::handle_quit_dialog_action(const std::string& id, const std::string& text)
{
    (void)text;

    if (_quit_dialog)
    {
        _quit_dialog->hide_dialog();
    }

    if (_screen)
    {
        _screen->set_input_enabled(true);
    }

    if (_menu_list)
    {
        _menu_list->set_active(true);
    }

    if (id == "quit")
    {
        if (_subtitle_label)
        {
            _subtitle_label->set_text("Closing...");
        }

        request_quit();
        return;
    }

    if (_subtitle_label)
    {
        _subtitle_label->set_text("Prototype Main Menu");
    }
}
