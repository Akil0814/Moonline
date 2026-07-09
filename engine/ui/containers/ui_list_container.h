#pragma once

#include "../focus/ui_control_focus_scope_host.h"
#include "../layout/ui_list_layout.h"

namespace elysia::ui
{
enum class UiListDirection
{
    Vertical,
    Horizontal
};

class UiListContainer : public UiControlFocusScopeHost
{
public:
    explicit UiListContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiListContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiListContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiListContainer() override = default;

    void reset() noexcept override;

    // Prepends a child to the list while preserving container ownership.
    void add_front(std::unique_ptr<UiElement> child);
    // Appends a child to the list while preserving container ownership.
    void add_back(std::unique_ptr<UiElement> child);
    UiElement* add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options) override;
    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override;

    void set_direction(UiListDirection direction) noexcept;
    [[nodiscard]] UiListDirection direction() const noexcept;
    void set_item_spacing(float item_spacing) noexcept;
    [[nodiscard]] float item_spacing() const noexcept;

protected:
    // Lays out children along the configured list direction and spacing.
    void rebuild_layout() override;
    // Refreshes focus neighbors after list ordering or geometry changes.
    void rebuild_focus_registry() override;

private:
    layout::UiListLayoutConfig _layout{};
};
}
