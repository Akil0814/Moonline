#pragma once

#include "../core/ui_child_host.h"
#include "../focus/ui_focus_scope.h"
#include "../core/ui_control.h"
#include "../../core/render/colors.h"
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

private:
    void prune_focus_scopes();
    void ensure_valid_scope_focus();
    void apply_scope_focus();
    void update_focus_input_device(elysia::input::InputDevice device) noexcept;
    bool restore_preferred_scope_focus();
    [[nodiscard]] UiFocusScope* find_registered_scope_at(int mouse_x,int mouse_y) const;
    [[nodiscard]] UiFocusScope* find_neighbor(const UiFocusScope& scope,UiAction action) const;
    [[nodiscard]] bool is_registered_scope(const UiFocusScope& scope) const noexcept;
    [[nodiscard]] bool set_focused_scope_internal(UiFocusScope* scope) noexcept;
    [[nodiscard]] bool dispatch_to_scope(UiFocusScope* scope,const UiInputEvent& event) const;
    [[nodiscard]] static bool is_scope_usable(const UiFocusScope* scope) noexcept;
    [[nodiscard]] static bool uses_pointer_focus_policy(elysia::input::InputDevice device) noexcept;

private:
    std::vector<ScopeEntry> _scope_entries;
    UiFocusScope* _focused_scope = nullptr;
    UiFocusScope* _last_focused_scope = nullptr;
    bool _draw_background = false;
    bool _draw_border = false;
    elysia::core::Color _background_color = elysia::core::colors::cobalt_blue;
    elysia::core::Color _border_color = elysia::core::colors::sky_blue;
    bool _hover_focus_enabled = true;
    elysia::input::InputDevice _focus_input_device = elysia::input::InputDevice::Unknown;
    UiWindowCancelCallback _on_cancel;
};
}
