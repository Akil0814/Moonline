#include "ui_menu_list.h"

#include "../style/ui_theme.h"

#include <utility>

UiMenuList::UiMenuList(Vector2 position, Vector2 size, int order)
    : UiSelectableScrollList(position, size, order)
{
}

void UiMenuList::reset()
{
    UiSelectableScrollList::reset();
    _items.clear();
    _buttons.clear();
    _item_size = { 320.0f, 56.0f };
    _font_key = "ui.default";
    _text_color = SDL_Color{ 255, 255, 255, 255 };
    _button_style = ButtonStyle{};
    _on_selection_changed = nullptr;
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
    clear_children();
    set_scroll_offset(Vector2::zero());
    set_selected_index(-1);
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

const UiMenuListItem* UiMenuList::selected_item() const
{
    if (selected_index() < 0 || selected_index() >= static_cast<int>(_items.size()))
    {
        return nullptr;
    }

    return &_items[static_cast<size_t>(selected_index())];
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

void UiMenuList::set_on_selection_changed(UiMenuSelectionChangedCallback on_selection_changed)
{
    _on_selection_changed = std::move(on_selection_changed);
}

bool UiMenuList::handle_focused_input_event(const InputEvent& event)
{
    if (handle_navigation_input_event(event))
    {
        return true;
    }

    if (!is_enabled() || !is_focused() || event.type != InputEventType::Pressed)
    {
        return false;
    }

    if (event.action != InputAction::Confirm)
    {
        return false;
    }

    const UiMenuListItem* item = selected_item();
    if (!item || !item->_enabled)
    {
        return true;
    }

    if (_on_selection_changed)
    {
        _on_selection_changed(selected_index(), item->_id, item->_text);
    }

    return true;
}

void UiMenuList::rebuild_items()
{
    clear_children();
    _buttons.clear();
    set_scroll_step({ _item_size.x + spacing(), _item_size.y + spacing() });

    for (const UiMenuListItem& item : _items)
    {
        std::shared_ptr<UiTextButton> button = std::make_shared<UiTextButton>(
            Vector2::zero(),
            _item_size
        );
        button->set_text(item._text);
        button->set_font_key(_font_key);
        button->set_text_color(_text_color);
        button->set_enabled(is_enabled() && item._enabled);
        UiStyle::apply_button(*button, _button_style);
        button->set_on_click([this, raw_button = button.get()]()
            {
                handle_item_click(raw_button);
            });

        UiLayoutChildOptions options;
        options._fill_cross_axis = true;
        add_child(button, options);
        _buttons.push_back(button);
    }

    if (_items.empty())
    {
        set_selected_index(-1);
        set_scroll_offset(Vector2::zero());
        return;
    }

    if (selected_index() < 0 || selected_index() >= static_cast<int>(_items.size()))
    {
        set_selected_index(0);
        return;
    }

    set_selected_index(selected_index());
}

void UiMenuList::sync_selection_state()
{
    for (size_t index = 0; index < _buttons.size(); ++index)
    {
        std::shared_ptr<UiTextButton>& button = _buttons[index];
        if (!button)
        {
            continue;
        }

        const bool enabled = is_enabled() && _items[index]._enabled;
        button->set_enabled(enabled);
        const bool focused = static_cast<int>(index) == selected_index();
        button->set_focused(is_focused() && enabled && focused);
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
        if (_on_selection_changed)
        {
            const UiMenuListItem& item = _items[index];
            _on_selection_changed(static_cast<int>(index), item._id, item._text);
        }
        return;
    }
}

size_t UiMenuList::selectable_item_count() const
{
    return _items.size();
}

bool UiMenuList::selectable_item_enabled(int index) const
{
    return index >= 0
        && index < static_cast<int>(_items.size())
        && _items[static_cast<size_t>(index)]._enabled;
}

const GameObject* UiMenuList::selected_item_view_target() const
{
    const int index = selected_index();
    if (index < 0 || index >= static_cast<int>(_buttons.size()))
    {
        return nullptr;
    }

    return _buttons[static_cast<size_t>(index)].get();
}

void UiMenuList::apply_theme(const UiTheme& theme)
{
    UiPanel::apply_theme(theme);
    _text_color = theme._default_label._text_color;
    _button_style = theme._primary_button;
    rebuild_items();
}
