#pragma once

#include "../focus/ui_control_focus_scope_host.h"
#include "../style/ui_visual_styles.h"

#include <vector>

namespace elysia::ui
{
class UiPopup : public UiControlFocusScopeHost
{
public:
    static constexpr int default_order = 1000;
    static constexpr float default_width = 360.0f;
    static constexpr float default_height = 220.0f;

public:
    UiPopup() noexcept;
    explicit UiPopup(const elysia::core::Rect& rect,int order = default_order) noexcept;
    UiPopup(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = default_order) noexcept;
    UiPopup(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = default_order) noexcept;
    ~UiPopup() override = default;

    void reset() noexcept override;

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_modal(bool modal) noexcept;
    [[nodiscard]] bool is_modal() const noexcept;
    void set_close_on_cancel(bool close_on_cancel) noexcept;
    [[nodiscard]] bool closes_on_cancel() const noexcept;
    void set_close_on_outside_click(bool close_on_outside_click) noexcept;
    [[nodiscard]] bool closes_on_outside_click() const noexcept;
    void close() noexcept;
    [[nodiscard]] bool is_open() const noexcept;

    void set_style(const UiPopupStyle& style) noexcept;
    [[nodiscard]] const UiPopupStyle& style() const noexcept;
    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;
    void set_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color background_color() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;

    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool should_close_from_event(const UiInputEvent& event) const noexcept;
    [[nodiscard]] bool uses_default_centering() const noexcept;

protected:
    void rebuild_layout() override;
    void rebuild_focus_registry() override;

private:
    [[nodiscard]] static constexpr elysia::core::Rect default_rect() noexcept
    {
        return elysia::core::Rect(0.0f,0.0f,default_width,default_height);
    }

private:
    UiPopupStyle _style{};
    bool _modal = true;
    bool _close_on_cancel = true;
    bool _close_on_outside_click = true;
    bool _use_default_centering = false;
};
}
