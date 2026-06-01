#include "ui_menu_list.h"

#include "../style/ui_theme.h"

#include <algorithm>
#include <utility>

UiMenuList::UiMenuList(Vector2 position, Vector2 size, int order)
    : UiScrollPanel(position, size, order)
{
    set_panel_theme_role(UiPanelThemeRole::List);
    set_direction(LayoutDirection::Vertical);
    set_allow_horizontal_scroll(false);
    set_allow_vertical_scroll(true);
    set_clamp_scroll(true);
}

void UiMenuList::on_input_event(const InputEvent& event)
{
    if (event.type == InputEventType::MouseWheel)
    {
        UiScrollPanel::on_input_event(event);
    }
}

void UiMenuList::reset()
{
    UiScrollPanel::reset();
    set_panel_theme_role(UiPanelThemeRole::List);
    set_direction(LayoutDirection::Vertical);
    set_allow_horizontal_scroll(false);
    set_allow_vertical_scroll(true);
    set_clamp_scroll(true);
    _items.clear();
    _buttons.clear();
    _item_size = { 320.0f, 56.0f };
    _font_key = "ui.default";
    _text_color = SDL_Color{ 255, 255, 255, 255 };
    _button_style = ButtonStyle{};
    _on_select = nullptr;
    _selected_index = -1;
    _enabled = true;
    _is_focused = false;
    clear_children();
}

int UiMenuList::add_item(const std::string& id, const std::string& text, bool enabled)
{
    UiMenuListItem item;
    item._id = id;
    item._text = text;
    item._enabled = enabled;
    _items.push_back(std::move(item));
    rebuild_items();
    return static_cast<int>(_items.size()) - 1;
}

void UiMenuList::clear_items()
{
    _items.clear();
    _buttons.clear();
    _selected_index = -1;
    clear_children();
    set_scroll_offset(Vector2::zero());
}

void UiMenuList::set_items(const std::vector<UiMenuListItem>& items)
{
    _items = items;
    rebuild_items();
}

size_t UiMenuList::item_count() const
{
    return _items.size();
}

int UiMenuList::selected_index() const
{
    return _selected_index;
}

void UiMenuList::set_selected_index(int index)
{
    if (_items.empty())
    {
        _selected_index = -1;
        sync_selection_visuals();
        return;
    }

    int clamped_index = std::clamp(index, 0, static_cast<int>(_items.size()) - 1);
    const int start_index = clamped_index;
    while (!_items[static_cast<size_t>(clamped_index)]._enabled)
    {
        clamped_index = (clamped_index + 1) % static_cast<int>(_items.size());
        if (clamped_index == start_index)
        {
            _selected_index = -1;
            sync_selection_visuals();
            return;
        }
    }

    _selected_index = clamped_index;
    sync_selection_visuals();

    if (_selected_index >= 0 && _selected_index < static_cast<int>(_buttons.size()))
    {
        ensure_child_visible(_buttons[static_cast<size_t>(_selected_index)].get());
    }
}

const UiMenuListItem* UiMenuList::selected_item() const
{
    if (_selected_index < 0 || _selected_index >= static_cast<int>(_items.size()))
    {
        return nullptr;
    }

    return &_items[static_cast<size_t>(_selected_index)];
}

bool UiMenuList::set_item_enabled(int index, bool enabled)
{
    if (index < 0 || index >= static_cast<int>(_items.size()))
    {
        return false;
    }

    _items[static_cast<size_t>(index)]._enabled = enabled;
    rebuild_items();
    return true;
}

bool UiMenuList::set_item_text(int index, const std::string& text)
{
    if (index < 0 || index >= static_cast<int>(_items.size()))
    {
        return false;
    }

    _items[static_cast<size_t>(index)]._text = text;
    rebuild_items();
    return true;
}

void UiMenuList::set_item_size(const Vector2& item_size)
{
    _item_size = item_size;
    rebuild_items();
}

const Vector2& UiMenuList::item_size() const
{
    return _item_size;
}

void UiMenuList::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    rebuild_items();
}

const std::string& UiMenuList::font_key() const
{
    return _font_key;
}

void UiMenuList::set_text_color(SDL_Color color)
{
    _text_color = color;
    rebuild_items();
}

SDL_Color UiMenuList::text_color() const
{
    return _text_color;
}

void UiMenuList::set_button_style(const ButtonStyle& button_style)
{
    _button_style = button_style;
    rebuild_items();
}

const ButtonStyle& UiMenuList::button_style() const
{
    return _button_style;
}

void UiMenuList::set_on_select(UiMenuSelectCallback on_select)
{
    _on_select = std::move(on_select);
}

void UiMenuList::rebuild_items()
{
    clear_children();
    _buttons.clear();

    for (size_t index = 0; index < _items.size(); ++index)
    {
        const UiMenuListItem& item = _items[index];

        std::shared_ptr<UiTextButton> button = std::make_shared<UiTextButton>(
            Vector2::zero(),
            _item_size
        );
        button->set_text(item._text);
        button->set_font_key(_font_key);
        button->set_text_color(_text_color);
        button->set_enabled(_enabled && item._enabled);
        UiStyle::apply_button(*button, _button_style);
        button->set_on_click([this, raw_button = button.get()]()
            {
                handle_item_click(raw_button);
            });

        LayoutChildOptions options;
        options._fill_cross_axis = true;
        add_child(button, options);
        _buttons.push_back(button);
    }

    if (_items.empty())
    {
        _selected_index = -1;
        set_scroll_offset(Vector2::zero());
        return;
    }

    if (_selected_index < 0 || _selected_index >= static_cast<int>(_items.size()))
    {
        _selected_index = 0;
    }

    set_selected_index(_selected_index);
}

void UiMenuList::sync_selection_visuals()
{
    for (size_t index = 0; index < _buttons.size(); ++index)
    {
        std::shared_ptr<UiTextButton>& button = _buttons[index];
        if (!button)
        {
            continue;
        }

        const bool enabled = _enabled && _items[index]._enabled;
        button->set_enabled(enabled);
        const bool focused = static_cast<int>(index) == _selected_index;
        button->set_focused(_is_focused && enabled && focused);
    }
}

void UiMenuList::handle_item_click(UiTextButton* button)
{
    if (!button)
    {
        return;
    }

    for (size_t index = 0; index < _buttons.size(); ++index)
    {
        if (_buttons[index].get() != button)
        {
            continue;
        }

        set_selected_index(static_cast<int>(index));

        if (_on_select)
        {
            const UiMenuListItem& item = _items[index];
            _on_select(static_cast<int>(index), item._id, item._text);
        }
        return;
    }
}

void UiMenuList::set_enabled(bool enabled)
{
    _enabled = enabled;
    sync_selection_visuals();
}

bool UiMenuList::is_enabled() const
{
    return _enabled;
}

void UiMenuList::set_focused(bool focused)
{
    _is_focused = focused;
    sync_selection_visuals();
}

bool UiMenuList::is_focused() const
{
    return _is_focused;
}

bool UiMenuList::handle_focused_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused)
    {
        return false;
    }

    if (event.type == InputEventType::MouseWheel)
    {
        UiScrollPanel::on_input_event(event);
        return true;
    }

    if (event.type != InputEventType::Pressed)
    {
        return false;
    }

    switch (event.action)
    {
    case InputAction::Up:
        set_selected_index(_selected_index - 1);
        return true;

    case InputAction::Down:
        set_selected_index(_selected_index + 1);
        return true;

    case InputAction::Confirm:
    {
        const UiMenuListItem* item = selected_item();
        if (!item || !item->_enabled)
        {
            return true;
        }

        if (_on_select)
        {
            _on_select(_selected_index, item->_id, item->_text);
        }
        return true;
    }

    default:
        return false;
    }
}

GameObject* UiMenuList::game_object()
{
    return this;
}

const GameObject* UiMenuList::game_object() const
{
    return this;
}

void UiMenuList::apply_theme(const UiTheme& theme)
{
    UiPanel::apply_theme(theme);
    _text_color = theme._default_label._text_color;
    _button_style = theme._primary_button;
    rebuild_items();
}
