#pragma once

#include "../focus/ui_control_focus_scope_host.h"
#include "../style/ui_visual_styles.h"

#include <vector>

namespace elysia::ui
{
enum class UiPanelInsertDirection
{
    Up,
    Down,
    Left,
    Right
};

class UiPanel : public UiControlFocusScopeHost
{
public:
    explicit UiPanel(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiPanel(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiPanel(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiPanel() override = default;

    void reset() noexcept override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;
    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override;

    // Inserts a child relative to the last panel insertion point and updates focus links.
    void add_child(std::unique_ptr<UiElement> child,UiPanelInsertDirection direction = UiPanelInsertDirection::Down);
    UiElement* add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options) override;

    void set_style(const UiPanelStyle& style) noexcept;
    [[nodiscard]] const UiPanelStyle& style() const noexcept;

    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;
    void set_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color background_color() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;

protected:
    // Repositions children using the panel's incremental insertion layout.
    void rebuild_layout() override;
    // Rebuilds directional focus neighbors based on insertion relationships.
    void rebuild_focus_registry() override;

private:
    // Stores directional navigation links for one focusable child in the panel flow.
    struct FocusLink
    {
        UiControl* control = nullptr;
        UiFocusNeighbors neighbors;
    };

private:
    // Performs directional child insertion and returns the adopted child pointer.
    UiElement* insert_panel_child(std::unique_ptr<UiElement> child,UiPanelInsertDirection direction);
    // Removes focus links that no longer reference live child controls.
    void prune_panel_links();
    // Returns or creates the focus-link record for a child control.
    FocusLink& ensure_link(UiControl& control);
    FocusLink* find_link(UiControl& control) noexcept;
    const FocusLink* find_link(const UiControl& control) const noexcept;

private:
    UiPanelStyle _style{};
    std::vector<FocusLink> _focus_links;
    UiControl* _last_focusable = nullptr;
    elysia::core::Vector2 _last_child_layout_origin{};
    bool _has_child_layout_origin = false;
};
}
