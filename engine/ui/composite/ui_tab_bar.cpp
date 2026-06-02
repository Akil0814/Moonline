#include "ui_tab_bar.h"

#include "../base/ui_selectable_index_utils.h"

UiTabBar::UiTabBar(Vector2 position, Vector2 size, int order)
    : UiPanel(position, size, order)
{
    _button_group.set_on_selection_changed(
        [this](int index, UiSelectableButton* button)
        {
            (void)button;
            handle_group_selection_changed(index, button);
        }
    );

    reset();
}

void UiTabBar::reset()
{
    UiPanel::reset();
    set_direction(UiLayoutDirection::Horizontal);
    set_anchor(UiLayoutAnchor::TopLeft);
    set_cross_align(UiLayoutAlign::Center);
    set_spacing(12.0f);
    set_padding({ 8.0f, 8.0f, 8.0f, 8.0f });
    set_draw_background(false);

    _items.clear();
    _buttons.clear();
    _button_group.clear_buttons();
    _on_selection_changed = nullptr;
    _tab_size = { 160.0f, 52.0f };
    _font_key = "ui.default";
    _button_style = ButtonStyle{};
    _button_theme_role = UiButtonThemeRole::Primary;
    _enabled = true;
    _is_focused = false;
    _has_button_style_override = false;
    clear_children();
}

int UiTabBar::add_tab(const std::string& id, const std::string& text, bool enabled)
{
    UiTabBarItem item;
    item._id = id;
    item._text = text;
    item._enabled = enabled;
    _items.push_back(std::move(item));
    rebuild_tabs();
    return static_cast<int>(_items.size()) - 1;
}

void UiTabBar::set_tabs(const std::vector<UiTabBarItem>& items)
{
    _items = items;
    rebuild_tabs();
}

void UiTabBar::clear_tabs()
{
    _items.clear();
    _buttons.clear();
    _button_group.clear_buttons();
    clear_children();
}

size_t UiTabBar::tab_count() const
{
    return _items.size();
}

int UiTabBar::selected_index() const
{
    return _button_group.selected_index();
}

void UiTabBar::set_selected_index(int index)
{
    if (_items.empty())
    {
        _button_group.set_selected_index(-1);
        sync_button_state();
        return;
    }

    const int resolved_index = ui_selectable_index_utils::resolve_index(
        index,
        _items.size(),
        [this](int item_index)
        {
            return _items[static_cast<size_t>(item_index)]._enabled;
        }
    );
    _button_group.set_selected_index(resolved_index);
}

const UiTabBarItem* UiTabBar::selected_tab() const
{
    const int index = selected_index();
    if (index < 0 || index >= static_cast<int>(_items.size()))
    {
        return nullptr;
    }

    return &_items[static_cast<size_t>(index)];
}

void UiTabBar::set_tab_size(const Vector2& tab_size)
{
    _tab_size = tab_size;
    refresh_tabs();
    sync_selection_index();
}

const Vector2& UiTabBar::tab_size() const
{
    return _tab_size;
}

void UiTabBar::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    refresh_tabs();
    sync_selection_index();
}

const std::string& UiTabBar::font_key() const
{
    return _font_key;
}

void UiTabBar::set_button_theme_role(UiButtonThemeRole button_theme_role)
{
    _button_theme_role = button_theme_role;
    refresh_tabs();
    sync_selection_index();
}

UiButtonThemeRole UiTabBar::button_theme_role() const
{
    return _button_theme_role;
}

void UiTabBar::set_button_style(const ButtonStyle& button_style)
{
    _button_style = button_style;
    _has_button_style_override = true;
    refresh_tabs();
    sync_selection_index();
}

const ButtonStyle& UiTabBar::button_style() const
{
    return _button_style;
}

void UiTabBar::clear_button_style_override()
{
    _button_style = ButtonStyle{};
    _has_button_style_override = false;
    refresh_tabs();
    sync_selection_index();
}

bool UiTabBar::has_button_style_override() const
{
    return _has_button_style_override;
}

void UiTabBar::set_on_selection_changed(UiTabBarSelectionChangedCallback on_selection_changed)
{
    _on_selection_changed = std::move(on_selection_changed);
}

void UiTabBar::set_enabled(bool enabled)
{
    _enabled = enabled;
    sync_button_state();
}

bool UiTabBar::is_enabled() const
{
    return _enabled;
}

void UiTabBar::set_focused(bool focused)
{
    _is_focused = focused;
    sync_button_state();
}

bool UiTabBar::is_focused() const
{
    return _is_focused;
}

bool UiTabBar::handle_focused_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused || event.type != InputEventType::Pressed || _items.empty())
    {
        return false;
    }

    switch (event.action)
    {
    case InputAction::Left:
        set_selected_index(selected_index() - 1);
        return true;

    case InputAction::Right:
        set_selected_index(selected_index() + 1);
        return true;

    case InputAction::Confirm:
        emit_selection_changed();
        return true;

    default:
        return false;
    }
}

GameObject* UiTabBar::game_object()
{
    return this;
}

const GameObject* UiTabBar::game_object() const
{
    return this;
}

void UiTabBar::rebuild_tabs()
{
    clear_children();
    _buttons.clear();
    _button_group.clear_buttons();

    for (size_t index = 0; index < _items.size(); ++index)
    {
        std::shared_ptr<UiSelectableButton> button = std::make_shared<UiSelectableButton>(
            Vector2::zero(),
            _tab_size
        );
        add_child(button);
        _button_group.add_button(button);
        _buttons.push_back(std::move(button));
    }

    refresh_tabs();
    sync_selection_index();
}

void UiTabBar::refresh_tabs()
{
    if (_buttons.size() != _items.size())
    {
        rebuild_tabs();
        return;
    }

    for (size_t index = 0; index < _buttons.size(); ++index)
    {
        refresh_tab(index);
    }

    sync_button_state();
}

void UiTabBar::refresh_tab(size_t index)
{
    if (index >= _buttons.size() || index >= _items.size())
    {
        return;
    }

    const std::shared_ptr<UiSelectableButton>& button = _buttons[index];
    if (!button)
    {
        return;
    }

    const UiTabBarItem& item = _items[index];
    button->set_size(_tab_size);
    button->set_text(item._text);
    button->set_font_key(_font_key);
    button->set_button_theme_role(_button_theme_role);
    if (_has_button_style_override)
    {
        UiStyle::apply_button(*button, _button_style);
    }
    else
    {
        button->use_theme_appearance();
    }

    UiLayoutChildOptions options;
    options._use_size_override = true;
    options._size_override = _tab_size;
    set_child_options(button.get(), options);
}

void UiTabBar::sync_selection_index()
{
    if (_items.empty())
    {
        _button_group.set_selected_index(-1, false);
        sync_button_state();
        return;
    }

    const int selection_seed = selected_index() >= 0
        ? selected_index()
        : 0;
    const int resolved_index = ui_selectable_index_utils::resolve_index(
        selection_seed,
        _items.size(),
        [this](int item_index)
        {
            return _items[static_cast<size_t>(item_index)]._enabled;
        }
    );
    _button_group.set_selected_index(resolved_index, false);
    sync_button_state();
}

void UiTabBar::sync_button_state()
{
    const int current_selected_index = selected_index();
    for (int index = 0; index < static_cast<int>(_buttons.size()); ++index)
    {
        const std::shared_ptr<UiSelectableButton>& button = _buttons[static_cast<size_t>(index)];
        if (!button)
        {
            continue;
        }

        const bool enabled = _enabled && _items[static_cast<size_t>(index)]._enabled;
        button->set_enabled(enabled);
        button->set_focused(enabled && _is_focused && index == current_selected_index);
    }
}

void UiTabBar::handle_group_selection_changed(int index, UiSelectableButton* button)
{
    (void)button;
    (void)index;
    sync_button_state();
    emit_selection_changed();
}

void UiTabBar::emit_selection_changed()
{
    if (!_on_selection_changed)
    {
        return;
    }

    const UiTabBarItem* item = selected_tab();
    if (!item)
    {
        return;
    }

    _on_selection_changed(selected_index(), item->_id, item->_text);
}
