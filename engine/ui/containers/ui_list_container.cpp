#include "ui_list_container.h"

namespace elysia::ui
{
namespace
{
[[nodiscard]] UiLayoutDirection to_layout_direction(UiListDirection direction) noexcept
{
    return direction == UiListDirection::Vertical ? UiLayoutDirection::Vertical : UiLayoutDirection::Horizontal;
}
}

UiListContainer::UiListContainer(const elysia::core::Rect& rect,int order) noexcept
    : UiControlFocusScopeHost(rect,order) {}

UiListContainer::UiListContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiControlFocusScopeHost(position,size,order) {}

UiListContainer::UiListContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiControlFocusScopeHost(center,size,from_center,order) {}

void UiListContainer::reset() noexcept
{
    UiControlFocusScopeHost::reset();
    _layout = layout::UiListLayoutConfig{};
}

void UiListContainer::add_front(std::unique_ptr<UiElement> child)
{
    (void)insert_child(std::move(child),0,UiLayoutChildOptions{});
}

void UiListContainer::add_back(std::unique_ptr<UiElement> child)
{
    (void)insert_child(std::move(child),child_count(),UiLayoutChildOptions{});
}

UiElement* UiListContainer::add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options)
{
    return insert_child(std::move(child),child_count(),options);
}

void UiListContainer::set_direction(UiListDirection direction) noexcept
{
    _layout.direction = to_layout_direction(direction);
    invalidate_intrinsic_layout();
}

UiListDirection UiListContainer::direction() const noexcept
{
    return _layout.direction == UiLayoutDirection::Vertical ? UiListDirection::Vertical : UiListDirection::Horizontal;
}

void UiListContainer::set_item_spacing(float item_spacing) noexcept
{
    _layout.item_spacing = layout::clamp_non_negative(item_spacing);
    invalidate_intrinsic_layout();
}

float UiListContainer::item_spacing() const noexcept
{
    return _layout.item_spacing;
}

void UiListContainer::rebuild_layout()
{
    layout::layout_list_children(children(),content_rect(),_layout);
}

void UiListContainer::rebuild_focus_registry()
{
    const std::vector<UiControl*> controls = direct_focusable_children();
    std::vector<FocusEntry> entries;
    entries.reserve(controls.size());

    for (std::size_t index = 0; index < controls.size(); ++index)
    {
        UiFocusNeighbors neighbors;
        if (_layout.direction == UiLayoutDirection::Vertical)
        {
            if (index > 0)
                neighbors.up = controls[index - 1];
            if (index + 1 < controls.size())
                neighbors.down = controls[index + 1];
        }
        else
        {
            if (index > 0)
                neighbors.left = controls[index - 1];
            if (index + 1 < controls.size())
                neighbors.right = controls[index + 1];
        }
        entries.push_back(FocusEntry{ controls[index],neighbors });
    }

    set_focus_entries(std::move(entries));
}
}
