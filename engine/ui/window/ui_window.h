#pragma once

#include "../core/ui_child_host.h"
#include "../core/ui_control.h"
#include "../../core/render/colors.h"

#include <functional>
#include <vector>

namespace elysia::ui
{
enum class UiWindowFocusSlot
{
    Custom,
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

struct UiWindowFocusNeighbors
{
    UiControl* up = nullptr;
    UiControl* down = nullptr;
    UiControl* left = nullptr;
    UiControl* right = nullptr;
};

struct UiWindowFocusOptions
{
    UiWindowFocusSlot slot = UiWindowFocusSlot::Custom;
    UiWindowFocusNeighbors neighbors;
};

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

    void register_focus_target(UiControl& control,const UiWindowFocusOptions& options = {});
    void unregister_focus_target(UiControl& control);
    void set_focus_neighbors(UiControl& control,const UiWindowFocusNeighbors& neighbors);
    void set_focused_target(UiControl* control);
    [[nodiscard]] UiControl* focused_target() const noexcept;
    bool focus_first_available();

    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

protected:
    void rebuild_layout() override;

private:
    struct FocusEntry
    {
        UiControl* control = nullptr;
        UiWindowFocusOptions options;
    };

private:
    void prune_focus_targets();
    void ensure_valid_focus();
    void apply_focus_state();
    [[nodiscard]] UiControl* find_registered_target_at(int mouse_x,int mouse_y) const;
    [[nodiscard]] UiControl* find_neighbor(const UiControl& control,UiAction action) const;
    [[nodiscard]] bool is_registered_focus_target(const UiControl& control) const noexcept;
    [[nodiscard]] bool set_focused_target_internal(UiControl* control) noexcept;
    [[nodiscard]] static bool is_control_usable(const UiControl* control) noexcept;

private:
    std::vector<FocusEntry> _focus_entries;
    UiControl* _focused_target = nullptr;
    bool _draw_background = false;
    bool _draw_border = false;
    elysia::core::Color _background_color = elysia::core::colors::cobalt_blue;
    elysia::core::Color _border_color = elysia::core::colors::sky_blue;
    bool _hover_focus_enabled = true;
    UiWindowCancelCallback _on_cancel;
};
}
