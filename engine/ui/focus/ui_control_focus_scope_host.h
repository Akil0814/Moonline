#pragma once

#include "ui_focus_scope.h"
#include "ui_focus_scope_utils.h"
#include "../core/ui_child_host.h"
#include "../core/ui_control.h"

#include <vector>

namespace elysia::ui
{
class UiControlFocusScopeHost : public UiChildHost, public UiFocusScope
{
public:
    struct FocusEntry
    {
        UiControl* control = nullptr;
        UiFocusNeighbors neighbors;
    };

public:
    explicit UiControlFocusScopeHost(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiControlFocusScopeHost(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiControlFocusScopeHost(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiControlFocusScopeHost() override = default;

    void reset() noexcept override;

    void set_focused_target(UiControl* control);
    [[nodiscard]] UiControl* focused_target() const noexcept override;
    bool focus_first_available() override;
    [[nodiscard]] bool has_focusable_target() const noexcept override;

    [[nodiscard]] UiElement& focus_scope_element() noexcept override;
    [[nodiscard]] const UiElement& focus_scope_element() const noexcept override;
    void set_scope_focused(bool focused) noexcept override;
    [[nodiscard]] bool is_scope_focused() const noexcept override;
    [[nodiscard]] bool contains_focus_point(int mouse_x,int mouse_y) const noexcept override;

    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

protected:
    virtual void rebuild_focus_registry() = 0;

    void refresh_focus_registry();
    void set_focus_entries(std::vector<FocusEntry> entries);
    [[nodiscard]] const std::vector<FocusEntry>& focus_entries() const noexcept;
    [[nodiscard]] std::vector<FocusEntry>& focus_entries() noexcept;
    [[nodiscard]] std::vector<UiControl*> direct_focusable_children() const;
    void ensure_valid_focus();
    void apply_focus_state() const;
    [[nodiscard]] UiControl* find_registered_target_at(int mouse_x,int mouse_y) const;
    [[nodiscard]] bool is_registered_focus_target(const UiControl& control) const noexcept;
    [[nodiscard]] bool set_focused_target_internal(UiControl* control) noexcept;

private:
    [[nodiscard]] UiControl* find_neighbor(const UiControl& control,UiAction action) const noexcept;

private:
    std::vector<FocusEntry> _focus_entries;
    UiControl* _focused_target = nullptr;
    bool _scope_focused = false;
};
}
