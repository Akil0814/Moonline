#pragma once

#include "../core/ui_child_host.h"
#include "../focus/ui_focus_scope.h"
#include "../scroll/ui_scroll_state.h"
#include "../widgets/ui_drag_handle.h"

namespace elysia::ui
{
enum class UiScrollBarVisibility
{
    Hidden,
    Auto,
    Always
};

struct UiScrollBarStyle
{
    float thickness = 10.0f;
    float margin = 4.0f;
    float min_thumb_length = 24.0f;
    bool draw_track = true;
    elysia::core::Color track_idle_color = UiPalette::scrollbar_track_idle;
    elysia::core::Color track_focused_color = UiPalette::scrollbar_track_focused;
    elysia::core::Color track_dragging_color = UiPalette::scrollbar_track_active;
    elysia::core::Color track_disabled_color = UiPalette::scrollbar_track_disabled;
    elysia::core::Color thumb_idle_color = UiPalette::scrollbar_thumb_idle;
    elysia::core::Color thumb_focused_color = UiPalette::scrollbar_thumb_focused;
    elysia::core::Color thumb_dragging_color = UiPalette::scrollbar_thumb_active;
    elysia::core::Color thumb_disabled_color = UiPalette::scrollbar_thumb_disabled;
};

struct UiScrollContainerStyle
{
    UiScrollBarStyle scrollbar{};
    bool draw_border = false;
    elysia::core::Color border_color = UiPalette::border_default;
};

class UiScrollContainer : public UiChildHost, public UiFocusScope
{
public:
    explicit UiScrollContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiScrollContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiScrollContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiScrollContainer() override = default;

    void reset() noexcept override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    UiElement* add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options = {}) override;
    UiElement* set_content(std::unique_ptr<UiElement> content);
    [[nodiscard]] UiElement* content() noexcept;
    [[nodiscard]] const UiElement* content() const noexcept;
    void clear_content();

    void set_scroll_axis(UiScrollAxis axis) noexcept;
    [[nodiscard]] UiScrollAxis scroll_axis() const noexcept;
    [[nodiscard]] UiScrollAxis resolved_scroll_axis() const noexcept;

    void set_style(const UiScrollContainerStyle& style) noexcept;
    [[nodiscard]] const UiScrollContainerStyle& style() const noexcept;

    void set_scrollbar_visibility(UiScrollBarVisibility visibility) noexcept;
    [[nodiscard]] UiScrollBarVisibility scrollbar_visibility() const noexcept;
    void set_scrollbar_style(const UiScrollBarStyle& style);
    [[nodiscard]] const UiScrollBarStyle& scrollbar_style() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;
    [[nodiscard]] elysia::core::Vector2 content_size() const noexcept;

    void set_scroll_offset(const elysia::core::Vector2& scroll_offset) noexcept;
    [[nodiscard]] elysia::core::Vector2 scroll_offset() const noexcept;
    void set_scroll_offset_x(float scroll_offset_x) noexcept;
    [[nodiscard]] float scroll_offset_x() const noexcept;
    void set_scroll_offset_y(float scroll_offset_y) noexcept;
    [[nodiscard]] float scroll_offset_y() const noexcept;
    [[nodiscard]] elysia::core::Vector2 max_scroll_offset() const noexcept;

    void set_scroll_step(const elysia::core::Vector2& scroll_step) noexcept;
    [[nodiscard]] elysia::core::Vector2 scroll_step() const noexcept;
    void set_scroll_step_x(float scroll_step_x) noexcept;
    [[nodiscard]] float scroll_step_x() const noexcept;
    void set_scroll_step_y(float scroll_step_y) noexcept;
    [[nodiscard]] float scroll_step_y() const noexcept;

    void scroll_by(const elysia::core::Vector2& delta) noexcept;
    void scroll_to_left() noexcept;
    void scroll_to_right() noexcept;
    void scroll_to_top() noexcept;
    void scroll_to_bottom() noexcept;
    void ensure_visible(const elysia::core::Rect& target_rect) noexcept;

    [[nodiscard]] UiElement& focus_scope_element() noexcept override;
    [[nodiscard]] const UiElement& focus_scope_element() const noexcept override;
    void set_scope_focused(bool focused) noexcept override;
    [[nodiscard]] bool is_scope_focused() const noexcept override;
    [[nodiscard]] bool has_focusable_target() const noexcept override;
    bool focus_first_available() override;
    [[nodiscard]] UiControl* focused_target() const noexcept override;
    [[nodiscard]] bool contains_focus_point(int mouse_x,int mouse_y) const noexcept override;

protected:
    void rebuild_layout() override;

private:
    [[nodiscard]] UiFocusScope* content_scope() noexcept;
    [[nodiscard]] const UiFocusScope* content_scope() const noexcept;
    [[nodiscard]] UiElement* set_content_internal(std::unique_ptr<UiElement> content);
    [[nodiscard]] bool dispatch_content_input_event(const UiInputEvent& event);
    [[nodiscard]] bool should_dispatch_content_input_event(const UiInputEvent& event) const noexcept;
    [[nodiscard]] bool should_dispatch_content_mouse_wheel(const UiInputEvent& event) const noexcept;
    [[nodiscard]] bool handle_mouse_wheel(const UiInputEvent& event);
    [[nodiscard]] bool dispatch_to_scrollbars(const UiInputEvent& event);
    [[nodiscard]] bool can_scroll_axis(UiScrollAxis axis) const noexcept;
    [[nodiscard]] bool shows_scrollbar(UiScrollAxis axis) const noexcept;
    [[nodiscard]] elysia::core::Rect interactive_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect viewport_rect() const noexcept;
    [[nodiscard]] bool is_pointer_in_interactive_rect(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_pointer_in_viewport(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] elysia::core::Rect scrollbar_track_rect(UiScrollAxis axis) const noexcept;
    [[nodiscard]] elysia::core::Rect scrollbar_thumb_rect(UiScrollAxis axis,const elysia::core::Rect& track_rect) const noexcept;
    [[nodiscard]] elysia::core::Color current_track_color(const UiDragHandle& thumb) const noexcept;
    void initialize_scrollbar_handles();
    void sync_scroll_state_to_viewport() noexcept;
    void sync_scroll_state_to_content() noexcept;
    void sync_scrollbar_handles() noexcept;
    void configure_scrollbar_thumb(
        UiDragHandle& thumb,
        UiScrollAxis axis,
        const elysia::core::Rect& track_rect,
        const elysia::core::Rect& thumb_rect
    ) noexcept;
    void update_horizontal_offset_from_thumb() noexcept;
    void update_vertical_offset_from_thumb() noexcept;
    void reset_scroll_offset() noexcept;
    void clear_content_pointer_state() noexcept;
    void sync_content_scope_focus() noexcept;
    void set_content_focus_suppressed(bool suppressed) noexcept;
    void update_content_focus_suppression(const UiInputEvent& event) noexcept;
    [[nodiscard]] bool should_auto_position_focus(const UiInputEvent& event) const noexcept;
    void ensure_visible_focused_target_for_input(const UiInputEvent& event) noexcept;
    void ensure_visible_focused_target() noexcept;
    void submit_scrollbar_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const;
    [[nodiscard]] elysia::core::Vector2 measured_content_size() const noexcept;

private:
    UiScrollState _scroll_state;
    UiScrollBarVisibility _scrollbar_visibility = UiScrollBarVisibility::Auto;
    UiScrollContainerStyle _style{};
    UiDragHandle _horizontal_thumb;
    UiDragHandle _vertical_thumb;
    bool _scope_focused = false;
    bool _content_pointer_active = false;
    bool _content_focus_suppressed = false;
};
}
