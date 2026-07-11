#include "ui_radio_group.h"

#include <utility>

namespace elysia::ui
{
UiRadioGroup::UiRadioGroup(const elysia::core::Rect& rect,int order) noexcept : UiListContainer(rect,order) {}
UiRadioGroup::UiRadioGroup(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiListContainer(position,size,order) {}
UiRadioGroup::UiRadioGroup(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiListContainer(center,size,from_center,order) {}

void UiRadioGroup::reset() noexcept
{
    UiListContainer::reset();
    _selected_item = nullptr;
    _on_selection_changed = {};
    _is_syncing_selection = false;
    _selection_notification_pending = false;
}
void UiRadioGroup::update(double delta) { sync_selection(true); UiListContainer::update(delta); sync_selection(true); }
void UiRadioGroup::on_ui_input_frame(const UiInputFrame& input) { sync_selection(true); UiListContainer::on_ui_input_frame(input); sync_selection(true); }
bool UiRadioGroup::on_ui_input_event(const UiInputEvent& event)
{
    sync_selection(true); const bool handled = UiListContainer::on_ui_input_event(event); sync_selection(true); return handled;
}
void UiRadioGroup::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out) const
{
    auto* self = const_cast<UiRadioGroup*>(this); self->cleanup_destroyed_children(); self->sync_selection(false);
    UiListContainer::submit_ui_render_commands(out);
}
std::optional<std::size_t> UiRadioGroup::selected_index() const noexcept { return find_radio_index(_selected_item); }
bool UiRadioGroup::set_selected_index(std::size_t index)
{
    cleanup_destroyed_children(); UiRadioItem* target = radio_item_at(index); if (!target) return false;
    for (std::size_t i = 0; i < child_count(); ++i) if (UiRadioItem* item = radio_item_at(i)) item->set_selected(item == target);
    sync_selection(true); return _selected_item == target;
}
void UiRadioGroup::set_on_selection_changed(UiRadioGroupSelectionChangedCallback callback) { _on_selection_changed = std::move(callback); }

void UiRadioGroup::sync_selection(bool notify)
{
    if (_is_syncing_selection) return;
    _is_syncing_selection = true;

    UiRadioItem* first_item = nullptr;
    UiRadioItem* first_selected = nullptr;
    UiRadioItem* focused_selected = nullptr;
    const UiControl* focused = UiListContainer::focused_target();
    for (std::size_t i = 0; i < child_count(); ++i)
    {
        UiRadioItem* item = radio_item_at(i); if (!item) continue;
        if (!first_item) first_item = item;
        if (item->is_selected() && !first_selected) first_selected = item;
        if (focused && &item->radio_item_element() == focused && item->is_selected()) focused_selected = item;
    }

    UiRadioItem* keep = focused_selected ? focused_selected : first_selected;
    if (!keep && first_item) { first_item->set_selected(true); keep = first_item; }
    for (std::size_t i = 0; i < child_count(); ++i)
        if (UiRadioItem* item = radio_item_at(i); item && item != keep && item->is_selected()) item->set_selected(false);

    UiRadioItem* previous = _selected_item;
    _selected_item = keep;
    _is_syncing_selection = false;
    const bool changed = previous != _selected_item;
    if (changed && !notify) _selection_notification_pending = true;
    if (notify && (changed || _selection_notification_pending))
    {
        _selection_notification_pending = false;
        if (_on_selection_changed) _on_selection_changed(selected_index());
    }
}
UiRadioItem* UiRadioGroup::radio_item_at(std::size_t index) const noexcept
{
    const UiElement* child = child_at(index);
    return child ? dynamic_cast<UiRadioItem*>(const_cast<UiElement*>(child)) : nullptr;
}
std::optional<std::size_t> UiRadioGroup::find_radio_index(const UiRadioItem* item) const noexcept
{
    if (!item) return std::nullopt;
    for (std::size_t i = 0; i < child_count(); ++i) if (radio_item_at(i) == item) return i;
    return std::nullopt;
}
}
