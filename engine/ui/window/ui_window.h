#pragma once

#include "../core/ui_child_host.h"
#include "../focus/ui_focus_scope.h"
#include "../core/ui_control.h"
#include "../style/ui_visual_styles.h"
#include "ui_overlay.h"
#include "../../input/input_types.h"

#include <functional>
#include <vector>

namespace elysia::ui
{
using UiWindowCancelCallback = std::function<void()>;

class UiWindow : public UiChildHost
{
public:
    explicit UiWindow(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiWindow(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiWindow(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiWindow() override = default;

    void reset() noexcept override;

    void set_style(const UiWindowStyle& style) noexcept;
    [[nodiscard]] const UiWindowStyle& style() const noexcept;

    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;
    void set_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color background_color() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;
    void set_hover_focus_enabled(bool enabled) noexcept;
    [[nodiscard]] bool hover_focus_enabled() const noexcept;
    void set_on_cancel(UiWindowCancelCallback on_cancel);

    void register_focus_scope(UiFocusScope& scope,const UiFocusScopeNeighbors& neighbors = {});
    void unregister_focus_scope(UiFocusScope& scope);
    void set_scope_neighbors(UiFocusScope& scope,const UiFocusScopeNeighbors& neighbors);
    void set_focused_scope(UiFocusScope* scope);
    [[nodiscard]] UiFocusScope* focused_scope() const noexcept;
    bool focus_first_available_scope();

    void register_overlay(UiElement& element,UiOverlayOptions options = {});
    void unregister_overlay(UiElement& element);
    void set_overlay_open(UiElement& element,bool open);
    [[nodiscard]] bool is_overlay_open(const UiElement& element) const noexcept;
    [[nodiscard]] UiOverlayOptions* overlay_options(UiElement& element) noexcept;

    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

protected:
    void rebuild_layout() override;

private:
    struct ScopeEntry
    {
        UiFocusScope* scope = nullptr;
        UiFocusScopeNeighbors neighbors;
    };

    struct OverlayEntry
    {
        UiElement* element = nullptr;
        UiOverlayOptions options;
    };

private:
    void prune_focus_scopes();
    void prune_overlays();
    void ensure_valid_scope_focus();
    void apply_scope_focus();
    void update_focus_input_device(elysia::input::InputDevice device) noexcept;
    bool restore_preferred_scope_focus();
    [[nodiscard]] OverlayEntry* active_overlay() noexcept;
    [[nodiscard]] const OverlayEntry* active_overlay() const noexcept;
    [[nodiscard]] OverlayEntry* active_modal_overlay() noexcept;
    [[nodiscard]] const OverlayEntry* active_modal_overlay() const noexcept;
    [[nodiscard]] OverlayEntry* find_overlay(UiElement& element) noexcept;
    [[nodiscard]] const OverlayEntry* find_overlay(const UiElement& element) const noexcept;
    [[nodiscard]] UiFocusScope* overlay_focus_scope(OverlayEntry& entry) noexcept;
    [[nodiscard]] const UiFocusScope* overlay_focus_scope(const OverlayEntry& entry) const noexcept;
    [[nodiscard]] bool focus_overlay(OverlayEntry& entry);
    [[nodiscard]] bool dispatch_to_overlay(OverlayEntry& entry,const UiInputEvent& event);
    void sync_overlay_visibility(OverlayEntry& entry) noexcept;
    void sync_overlay_visibility_all() noexcept;
    void apply_overlay_placements() noexcept;
    void apply_overlay_placement(OverlayEntry& entry) noexcept;
    [[nodiscard]] bool should_close_overlay_from_event(const OverlayEntry& entry,const UiInputEvent& event) const noexcept;
    [[nodiscard]] bool contains_overlay_point(const OverlayEntry& entry,int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_live_child_element(const UiElement& element) const noexcept;
    [[nodiscard]] UiFocusScope* find_registered_scope_at(int mouse_x,int mouse_y) const;
    [[nodiscard]] UiFocusScope* find_neighbor(const UiFocusScope& scope,UiAction action) const;
    [[nodiscard]] bool is_registered_scope(const UiFocusScope& scope) const noexcept;
    [[nodiscard]] bool set_focused_scope_internal(UiFocusScope* scope) noexcept;
    [[nodiscard]] bool dispatch_to_scope(UiFocusScope* scope,const UiInputEvent& event) const;
    [[nodiscard]] static bool is_scope_usable(const UiFocusScope* scope) noexcept;
    [[nodiscard]] static bool uses_pointer_focus_policy(elysia::input::InputDevice device) noexcept;

private:
    std::vector<ScopeEntry> _scope_entries;
    std::vector<OverlayEntry> _overlay_entries;
    UiFocusScope* _focused_scope = nullptr;
    UiFocusScope* _last_focused_scope = nullptr;
    UiWindowStyle _style{};
    bool _hover_focus_enabled = true;
    elysia::input::InputDevice _focus_input_device = elysia::input::InputDevice::Unknown;
    UiWindowCancelCallback _on_cancel;
};
}
