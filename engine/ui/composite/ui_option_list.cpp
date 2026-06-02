#include "ui_option_list.h"

#include "../ui_mouse_utils.h"
#include "../style/ui_theme.h"

#include <SDL.h>

#include <algorithm>
#include <iomanip>
#include <sstream>

UiOptionList::UiOptionList(Vector2 position, Vector2 size, int order)
    : UiSelectableScrollList(position, size, order)
{
    set_anchor(UiLayoutAnchor::TopCenter);
}

void UiOptionList::on_input(const InputSnapshot& input)
{
    if (ui_mouse_input_allowed(input))
    {
        UiScrollPanel::on_input(input);
    }

    if (!is_enabled() || !ui_mouse_input_allowed(input))
    {
        _was_mouse_down = false;
        return;
    }

    const UiMouseState mouse_state = ui_current_mouse_state();

    if (ui_left_mouse_released(mouse_state, _was_mouse_down))
    {
        for (size_t index = 0; index < _rows.size(); ++index)
        {
            if (!_rows[index]._panel)
            {
                continue;
            }

            if (ui_contains_point(_rows[index]._panel->rect(), mouse_state._position))
            {
                set_selected_index(static_cast<int>(index));
                break;
            }
        }
    }

    _was_mouse_down = mouse_state._left_button_down;
}

void UiOptionList::reset()
{
    UiSelectableScrollList::reset();
    set_anchor(UiLayoutAnchor::TopCenter);
    _items.clear();
    _rows.clear();
    _font_key = "ui.default";
    _style = UiOptionListStyle{};
    _on_value_changed = nullptr;
    _was_mouse_down = false;
    clear_children();
}

void UiOptionList::set_items(const std::vector<UiOptionListItem>& items)
{
    _items = items;
    rebuild_rows();
}

int UiOptionList::add_item(const UiOptionListItem& item)
{
    _items.push_back(item);
    rebuild_rows();
    return static_cast<int>(_items.size()) - 1;
}

void UiOptionList::clear_items()
{
    _items.clear();
    _rows.clear();
    clear_children();
    set_scroll_offset(Vector2::zero());
    set_selected_index(-1);
}

size_t UiOptionList::item_count() const
{
    return _items.size();
}

const UiOptionListItem* UiOptionList::selected_item() const
{
    if (selected_index() < 0 || selected_index() >= static_cast<int>(_items.size()))
    {
        return nullptr;
    }

    return &_items[static_cast<size_t>(selected_index())];
}

bool UiOptionList::set_item_enabled(int index, bool enabled)
{
    if (index < 0 || index >= static_cast<int>(_items.size()))
    {
        return false;
    }

    _items[static_cast<size_t>(index)]._enabled = enabled;
    refresh_rows();
    sync_selection_index();
    return true;
}

bool UiOptionList::set_item_toggle_value(int index, bool value)
{
    if (index < 0 || index >= static_cast<int>(_items.size()))
    {
        return false;
    }

    UiOptionListItem& item = _items[static_cast<size_t>(index)];
    if (item._control_type != UiOptionControlType::Toggle)
    {
        return false;
    }

    item._toggle._value = value;
    refresh_row(static_cast<size_t>(index));
    sync_selection_state();
    emit_value_changed(index);
    return true;
}

bool UiOptionList::set_item_slider_value(int index, float value)
{
    if (index < 0 || index >= static_cast<int>(_items.size()))
    {
        return false;
    }

    UiOptionListItem& item = _items[static_cast<size_t>(index)];
    if (item._control_type != UiOptionControlType::Slider)
    {
        return false;
    }

    item._slider._value = std::clamp(value, item._slider._min_value, item._slider._max_value);
    refresh_row(static_cast<size_t>(index));
    sync_selection_state();
    emit_value_changed(index);
    return true;
}

void UiOptionList::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    refresh_rows();
    sync_selection_index();
}

const std::string& UiOptionList::font_key() const
{
    return _font_key;
}

void UiOptionList::set_style(const UiOptionListStyle& style)
{
    _style = style;
    refresh_rows();
    sync_selection_index();
}

const UiOptionListStyle& UiOptionList::style() const
{
    return _style;
}

void UiOptionList::set_on_value_changed(UiOptionValueChangedCallback on_value_changed)
{
    _on_value_changed = std::move(on_value_changed);
}

bool UiOptionList::handle_focused_input_event(const InputEvent& event)
{
    if (handle_navigation_input_event(event))
    {
        return true;
    }

    if (!is_enabled() || !is_focused() || event.type != InputEventType::Pressed)
    {
        return false;
    }

    switch (event.action)
    {
    case InputAction::Left:
    case InputAction::Right:
    case InputAction::Confirm:
        if (selected_index() >= 0 && selected_index() < static_cast<int>(_rows.size()))
        {
            RowWidgets& row = _rows[static_cast<size_t>(selected_index())];
            if (row._control && row._control->handle_focused_input_event(event))
            {
                sync_selection_state();
                return true;
            }
        }
        return false;

    default:
        return false;
    }
}

void UiOptionList::rebuild_rows()
{
    clear_children();
    _rows.clear();

    apply_layout_metrics();

    for (size_t index = 0; index < _items.size(); ++index)
    {
        const UiOptionListItem& item = _items[index];
        RowWidgets row;

        row._panel = std::make_shared<UiPanel>(Vector2::zero(), _style._row_size);
        row._label = std::make_shared<UiLabel>();
        row._panel->add_child(row._label);

        if (item._control_type == UiOptionControlType::Slider)
        {
            std::shared_ptr<UiSlider> slider = std::make_shared<UiSlider>(Vector2::zero(), _style._control_size);
            slider->set_on_value_changed(
                [this, item_index = static_cast<int>(index)](float value)
                {
                    UiOptionListItem& target_item = _items[static_cast<size_t>(item_index)];
                    target_item._slider._value = value;
                    emit_value_changed(item_index);
                }
            );
            row._control_object = slider;
            row._control = slider;
            row._slider = slider;
        }
        else
        {
            std::shared_ptr<UiToggle> toggle = std::make_shared<UiToggle>(Vector2::zero(), _style._control_size);
            toggle->set_on_changed(
                [this, item_index = static_cast<int>(index)](bool value)
                {
                    UiOptionListItem& target_item = _items[static_cast<size_t>(item_index)];
                    target_item._toggle._value = value;
                    emit_value_changed(item_index);
                }
            );
            row._control_object = toggle;
            row._control = toggle;
            row._toggle = toggle;
        }

        row._panel->add_child(row._control_object);
        add_child(row._panel);

        _rows.push_back(std::move(row));
    }

    refresh_rows();
    sync_selection_index();
}

void UiOptionList::apply_layout_metrics()
{
    set_spacing(_style._row_spacing);
    set_padding({
        _style._panel_padding,
        _style._panel_padding,
        _style._panel_padding,
        _style._panel_padding
    });
    set_scroll_step({ _style._row_size.x, _style._row_size.y + _style._row_spacing });
}

void UiOptionList::refresh_rows()
{
    if (_rows.size() != _items.size())
    {
        rebuild_rows();
        return;
    }

    apply_layout_metrics();
    for (size_t index = 0; index < _rows.size(); ++index)
    {
        refresh_row(index);
    }

    sync_selection_state();
}

void UiOptionList::refresh_row(size_t index)
{
    if (index >= _rows.size() || index >= _items.size())
    {
        return;
    }

    RowWidgets& row = _rows[index];
    const UiOptionListItem& item = _items[index];
    if (!row._panel || !row._label || !row._control_object || !row._control)
    {
        return;
    }

    row._panel->set_use_theme(false);
    row._panel->set_size(_style._row_size);
    row._panel->set_direction(UiLayoutDirection::Horizontal);
    row._panel->set_cross_align(UiLayoutAlign::Center);
    row._panel->set_spacing(16.0f);
    row._panel->set_padding({ 18.0f, 12.0f, 18.0f, 12.0f });

    row._label->set_use_theme(false);
    row._label->set_font_key(_font_key);
    row._label->set_text(item._label);
    row._label->set_auto_size(false);
    row._label->set_size({ _style._row_size.x * 0.46f, _style._row_size.y - 24.0f });
    row._label->set_horizontal_align(TextHorizontalAlign::Left);
    row._label->set_vertical_align(TextVerticalAlign::Center);

    UiLayoutChildOptions label_options;
    label_options._use_size_override = true;
    label_options._size_override = row._label->size();
    label_options._fill_cross_axis = true;
    row._panel->set_child_options(row._label.get(), label_options);

    if (row._slider)
    {
        row._slider->set_size(_style._control_size);
        row._slider->set_font_key(_font_key);
        row._slider->set_range(item._slider._min_value, item._slider._max_value);
        row._slider->set_value(item._slider._value);
        row._slider->set_step(item._slider._step);
        row._slider->set_value_precision(item._slider._display_precision);
        row._slider->set_value_suffix(item._slider._suffix);
        row._slider->set_show_value_text(true);
        UiStyle::apply_slider(*row._slider, _style._slider_style);
    }
    else if (row._toggle)
    {
        row._toggle->set_size(_style._control_size);
        row._toggle->set_font_key(_font_key);
        row._toggle->set_state_texts(item._toggle._off_text, item._toggle._on_text);
        row._toggle->set_value(item._toggle._value);
        UiStyle::apply_toggle(*row._toggle, _style._toggle_style);
    }

    UiLayoutChildOptions control_options;
    control_options._use_size_override = true;
    control_options._size_override = _style._control_size;
    control_options._use_custom_cross_align = true;
    control_options._cross_align = UiLayoutAlign::Center;
    row._panel->set_child_options(row._control_object.get(), control_options);

    UiLayoutChildOptions row_options;
    row_options._use_size_override = true;
    row_options._size_override = _style._row_size;
    set_child_options(row._panel.get(), row_options);
}

void UiOptionList::sync_selection_index()
{
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

void UiOptionList::sync_selection_state()
{
    for (size_t index = 0; index < _rows.size(); ++index)
    {
        if (!_rows[index]._panel || !_rows[index]._label || !_rows[index]._control)
        {
            continue;
        }

        const UiOptionListItem& item = _items[index];
        const bool is_selected = static_cast<int>(index) == selected_index();
        const bool row_enabled = is_enabled() && item._enabled;

        PanelStyle panel_style;
        panel_style._draw_background = true;
        panel_style._draw_border = true;
        panel_style._border_color = _style._row_border_color;
        panel_style._background_color = row_enabled
            ? (is_selected ? _style._row_selected_background_color : _style._row_background_color)
            : _style._row_disabled_background_color;
        UiStyle::apply_panel(*_rows[index]._panel, panel_style);

        LabelStyle label_style;
        label_style._horizontal_align = TextHorizontalAlign::Left;
        label_style._vertical_align = TextVerticalAlign::Center;
        label_style._text_color = row_enabled
            ? _style._label_text_color
            : _style._disabled_text_color;
        UiStyle::apply_label(*_rows[index]._label, label_style);

        _rows[index]._control->set_enabled(row_enabled);
        _rows[index]._control->set_focused(row_enabled && is_focused() && is_selected);
    }
}

void UiOptionList::emit_value_changed(int index)
{
    if (!_on_value_changed || index < 0 || index >= static_cast<int>(_items.size()))
    {
        return;
    }

    const UiOptionListItem& item = _items[static_cast<size_t>(index)];
    _on_value_changed(index, item._id, value_text(item));
}

std::string UiOptionList::value_text(const UiOptionListItem& item) const
{
    if (item._control_type == UiOptionControlType::Toggle)
    {
        return item._toggle._value ? item._toggle._on_text : item._toggle._off_text;
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(item._slider._display_precision) << item._slider._value;
    stream << item._slider._suffix;
    return stream.str();
}

size_t UiOptionList::selectable_item_count() const
{
    return _items.size();
}

bool UiOptionList::selectable_item_enabled(int index) const
{
    return index >= 0
        && index < static_cast<int>(_items.size())
        && _items[static_cast<size_t>(index)]._enabled;
}

const GameObject* UiOptionList::selected_item_view_target() const
{
    const int index = selected_index();
    if (index < 0 || index >= static_cast<int>(_rows.size()) || !_rows[static_cast<size_t>(index)]._panel)
    {
        return nullptr;
    }

    return _rows[static_cast<size_t>(index)]._panel.get();
}

void UiOptionList::apply_theme(const UiTheme& theme)
{
    UiPanel::apply_theme(theme);
    _style._row_background_color = theme._default_panel._background_color;
    _style._row_selected_background_color = theme._primary_button._hovered_color;
    _style._row_disabled_background_color = SDL_Color{ 10, 18, 30, 180 };
    _style._row_border_color = theme._list_panel._border_color;
    _style._label_text_color = theme._default_label._text_color;
    _style._disabled_text_color = theme._muted_label._text_color;
    _style._toggle_style = theme._toggle;
    _style._slider_style = theme._slider;
    refresh_rows();
    sync_selection_index();
}
