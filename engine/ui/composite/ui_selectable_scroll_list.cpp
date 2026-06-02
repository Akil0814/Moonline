#include "ui_selectable_scroll_list.h"

#include "../base/ui_selectable_index_utils.h"

#include <algorithm>

UiSelectableScrollList::UiSelectableScrollList(Vector2 position, Vector2 size, int order)
    : UiScrollPanel(position, size, order)
{
    set_panel_theme_role(UiPanelThemeRole::List);
    set_direction(UiLayoutDirection::Vertical);
    set_allow_horizontal_scroll(false);
    set_allow_vertical_scroll(true);
    set_clamp_scroll(true);
}

void UiSelectableScrollList::on_input_event(const InputEvent& event)
{
    if (event.type == InputEventType::MouseWheel && event.device == InputDevice::Mouse)
    {
        UiScrollPanel::on_input_event(event);
    }
}

void UiSelectableScrollList::reset()
{
    UiScrollPanel::reset();
    set_panel_theme_role(UiPanelThemeRole::List);
    set_direction(UiLayoutDirection::Vertical);
    set_allow_horizontal_scroll(false);
    set_allow_vertical_scroll(true);
    set_clamp_scroll(true);
    _selection_view_padding = 24.0f;
    _selected_index = -1;
    _enabled = true;
    _is_focused = false;
}

int UiSelectableScrollList::selected_index() const
{
    return _selected_index;
}

void UiSelectableScrollList::set_selected_index(int index)
{
    const size_t item_count = selectable_item_count();
    if (item_count == 0)
    {
        _selected_index = -1;
        sync_selection_state();
        return;
    }

    const int clamped_index = std::clamp(index, 0, static_cast<int>(item_count) - 1);
    _selected_index = ui_selectable_index_utils::resolve_index(
        clamped_index,
        item_count,
        [this](int selectable_index)
        {
            return selectable_item_enabled(selectable_index);
        }
    );
    sync_selection_state();
    sync_selection_view();
}

void UiSelectableScrollList::set_selection_view_padding(float selection_view_padding)
{
    _selection_view_padding = std::max(0.0f, selection_view_padding);
    sync_selection_view();
}

float UiSelectableScrollList::selection_view_padding() const
{
    return _selection_view_padding;
}

void UiSelectableScrollList::set_enabled(bool enabled)
{
    _enabled = enabled;
    sync_selection_state();
}

bool UiSelectableScrollList::is_enabled() const
{
    return _enabled;
}

void UiSelectableScrollList::set_focused(bool focused)
{
    _is_focused = focused;
    sync_selection_state();
}

bool UiSelectableScrollList::is_focused() const
{
    return _is_focused;
}

GameObject* UiSelectableScrollList::game_object()
{
    return this;
}

const GameObject* UiSelectableScrollList::game_object() const
{
    return this;
}

bool UiSelectableScrollList::handle_navigation_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused)
    {
        return false;
    }

    if (event.type == InputEventType::MouseWheel)
    {
        if (event.device == InputDevice::Gamepad)
        {
            scroll_by({
                -static_cast<float>(event.wheel_x) * scroll_step().x,
                -static_cast<float>(event.wheel_y) * scroll_step().y
            });
        }
        else
        {
            UiScrollPanel::on_input_event(event);
        }
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

    default:
        return false;
    }
}

void UiSelectableScrollList::sync_selection_index()
{
    if (selectable_item_count() == 0)
    {
        set_selected_index(-1);
        set_scroll_offset(Vector2::zero());
        return;
    }

    if (_selected_index < 0 || _selected_index >= static_cast<int>(selectable_item_count()))
    {
        set_selected_index(0);
        return;
    }

    set_selected_index(_selected_index);
}

void UiSelectableScrollList::sync_selection_view()
{
    const GameObject* target = selected_item_view_target();
    if (!target)
    {
        return;
    }

    ensure_child_visible(target, { 0.0f, _selection_view_padding });
}
