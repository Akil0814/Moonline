#include "ui_dialog.h"

#include "../style/ui_style.h"

UiDialog::UiDialog(Vector2 position, Vector2 size, int order)
    : UiScreen(position, size, order)
{
    ensure_controls();
    reset();
}

void UiDialog::on_input_event(const InputEvent& event)
{
    if (event.type == InputEventType::Pressed && event.action == InputAction::Cancel)
    {
        for (const UiDialogAction& action : _actions)
        {
            if (action._id != _cancel_action_id)
            {
                continue;
            }

            handle_action(-1, action._id, action._text);
            return;
        }

        hide_dialog();
        return;
    }

    UiScreen::on_input_event(event);
}

void UiDialog::reset()
{
    UiScreen::reset();
    set_size({ 520.0f, 320.0f });
    set_anchor(LayoutAnchor::Center);
    set_cross_align(LayoutAlign::Center);
    set_spacing(14.0f);
    set_padding({ 28.0f, 24.0f, 28.0f, 24.0f });
    set_transition_enabled(true);
    set_open(false);

    PanelStyle panel_style;
    panel_style._background_color = SDL_Color{ 18, 26, 40, 245 };
    panel_style._border_color = SDL_Color{ 120, 150, 192, 255 };
    panel_style._draw_background = true;
    panel_style._draw_border = true;
    UiStyle::apply_panel(*this, panel_style);

    if (_title_label)
    {
        _title_label->reset();
        _title_label->set_font_key("ui.default");
        _title_label->set_auto_size(true);

        LabelStyle title_style;
        title_style._text_color = SDL_Color{ 248, 248, 252, 255 };
        title_style._horizontal_align = TextHorizontalAlign::Center;
        title_style._vertical_align = TextVerticalAlign::Center;
        UiStyle::apply_label(*_title_label, title_style);
    }

    if (_message_label)
    {
        _message_label->reset();
        _message_label->set_font_key("ui.default");
        _message_label->set_auto_size(false);
        _message_label->set_size({ 420.0f, 88.0f });
        _message_label->set_wrap_width(420);

        LabelStyle message_style;
        message_style._text_color = SDL_Color{ 190, 206, 226, 255 };
        message_style._horizontal_align = TextHorizontalAlign::Center;
        message_style._vertical_align = TextVerticalAlign::Center;
        UiStyle::apply_label(*_message_label, message_style);
    }

    if (_action_list)
    {
        _action_list->reset();
        _action_list->set_size({ 360.0f, 120.0f });
        _action_list->set_item_size({ 300.0f, 50.0f });
        _action_list->set_font_key("ui.default");
        _action_list->set_text_color(SDL_Color{ 248, 248, 252, 255 });

        PanelStyle list_style;
        list_style._background_color = SDL_Color{ 24, 34, 52, 220 };
        list_style._border_color = SDL_Color{ 108, 136, 176, 255 };
        list_style._draw_background = true;
        list_style._draw_border = true;
        UiStyle::apply_panel(*_action_list, list_style);

        ButtonStyle button_style;
        button_style._idle_color = SDL_Color{ 34, 48, 72, 255 };
        button_style._hovered_color = SDL_Color{ 58, 84, 124, 255 };
        button_style._pushed_color = SDL_Color{ 24, 38, 58, 255 };
        button_style._frame_color = SDL_Color{ 108, 136, 176, 255 };
        _action_list->set_button_style(button_style);
        _action_list->set_on_select(
            [this](int index, const std::string& id, const std::string& text)
            {
                handle_action(index, id, text);
            }
        );
    }

    if (_action_scroll_bar)
    {
        _action_scroll_bar->reset();
        _action_scroll_bar->set_target(_action_list.get());

        ScrollBarStyle scroll_bar_style;
        scroll_bar_style._track_color = SDL_Color{ 16, 22, 34, 180 };
        scroll_bar_style._thumb_color = SDL_Color{ 120, 154, 206, 255 };
        scroll_bar_style._thickness = 8.0f;
        scroll_bar_style._target_margin = 6.0f;
        scroll_bar_style._min_thumb_size = 20.0f;
        scroll_bar_style._auto_hide = true;
        UiStyle::apply_scroll_bar(*_action_scroll_bar, scroll_bar_style);
    }

    rebuild();
}

void UiDialog::set_title(const std::string& title)
{
    _title = title;
    rebuild();
}

const std::string& UiDialog::title() const
{
    return _title;
}

void UiDialog::set_message(const std::string& message)
{
    _message = message;
    rebuild();
}

const std::string& UiDialog::message() const
{
    return _message;
}

void UiDialog::set_actions(const std::vector<UiDialogAction>& actions)
{
    _actions = actions;
    rebuild();
}

void UiDialog::set_cancel_action_id(const std::string& cancel_action_id)
{
    _cancel_action_id = cancel_action_id;
}

const std::string& UiDialog::cancel_action_id() const
{
    return _cancel_action_id;
}

void UiDialog::set_on_action(UiDialogActionCallback on_action)
{
    _on_action = std::move(on_action);
}

void UiDialog::show_dialog()
{
    open();
}

void UiDialog::hide_dialog()
{
    close();
}

void UiDialog::ensure_controls()
{
    if (!_title_label)
    {
        _title_label = std::make_shared<Label>();
    }

    if (!_message_label)
    {
        _message_label = std::make_shared<Label>();
    }

    if (!_action_list)
    {
        _action_list = std::make_shared<UiMenuList>();
    }

    if (!_action_scroll_bar)
    {
        _action_scroll_bar = std::make_shared<UiScrollBar>();
    }
}

void UiDialog::rebuild()
{
    clear_children();
    clear_focusable_controls();
    ensure_controls();

    if (_title_label)
    {
        _title_label->set_text(_title);
    }

    if (_message_label)
    {
        _message_label->set_text(_message);
    }

    if (_action_list)
    {
        std::vector<UiMenuListItem> items;
        items.reserve(_actions.size());
        for (const UiDialogAction& action : _actions)
        {
            items.push_back({ action._id, action._text, true });
        }

        _action_list->set_items(items);
        _action_list->set_selected_index(0);
    }

    LayoutChildOptions centered_options;
    centered_options._use_custom_cross_align = true;
    centered_options._cross_align = LayoutAlign::Center;

    LayoutChildOptions message_options = centered_options;
    message_options._margin.top = 6.0f;
    message_options._margin.bottom = 8.0f;

    add_child(_title_label, centered_options);
    add_child(_message_label, message_options);
    add_child(_action_list, centered_options);
    if (_action_list)
    {
        register_focusable_control(_action_list);
        set_focused_control(0);
    }
}

void UiDialog::handle_action(
    int index,
    const std::string& id,
    const std::string& text
)
{
    (void)index;

    if (_on_action)
    {
        _on_action(id, text);
    }
}
