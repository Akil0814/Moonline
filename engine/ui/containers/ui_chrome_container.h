#pragma once

#include "../focus/ui_control_focus_scope_host.h"
#include "../style/ui_visual_styles.h"
#include "ui_list_container.h"

#include <memory>
#include <vector>

namespace elysia::ui
{
class UiChromeContainer : public UiControlFocusScopeHost
{
private:
    class SlotHost : public UiChildHost
    {
    public:
        explicit SlotHost(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
        void set_fill_children(bool fill_children) noexcept;

    protected:
        void rebuild_layout() override;

    private:
        bool _fill_children = false;
    };

public:
    explicit UiChromeContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiChromeContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiChromeContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiChromeContainer() override = default;

    void reset() noexcept override;
    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;
    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override;
    void set_scope_focused(bool focused) noexcept override;
    [[nodiscard]] bool is_scope_focused() const noexcept override;
    [[nodiscard]] bool has_focusable_target() const noexcept override;
    bool focus_first_available() override;
    [[nodiscard]] UiControl* focused_target() const noexcept override;
    [[nodiscard]] bool can_navigate(UiAction action) const noexcept override;

    [[nodiscard]] UiListContainer& left_actions() noexcept;
    [[nodiscard]] const UiListContainer& left_actions() const noexcept;
    [[nodiscard]] UiChildHost& title_slot() noexcept;
    [[nodiscard]] const UiChildHost& title_slot() const noexcept;
    [[nodiscard]] UiListContainer& right_actions() noexcept;
    [[nodiscard]] const UiListContainer& right_actions() const noexcept;
    [[nodiscard]] UiChildHost& body() noexcept;
    [[nodiscard]] const UiChildHost& body() const noexcept;

    UiElement* set_body(std::unique_ptr<UiElement> body);
    [[nodiscard]] UiElement* body_content() noexcept;
    [[nodiscard]] const UiElement* body_content() const noexcept;
    void clear_body();

    void set_header_visible(bool visible) noexcept;
    [[nodiscard]] bool header_visible() const noexcept;
    void set_header_height(float height) noexcept;
    [[nodiscard]] float header_height() const noexcept;
    void set_header_padding(const UiLayoutPadding& padding) noexcept;
    [[nodiscard]] const UiLayoutPadding& header_padding() const noexcept;
    void set_body_padding(const UiLayoutPadding& padding) noexcept;
    [[nodiscard]] const UiLayoutPadding& body_padding() const noexcept;

    void set_style(const UiChromeContainerStyle& style) noexcept;
    [[nodiscard]] const UiChromeContainerStyle& style() const noexcept;
    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;
    void set_draw_header_background(bool draw_header_background) noexcept;
    [[nodiscard]] bool draws_header_background() const noexcept;
    void set_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color background_color() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;
    void set_header_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color header_background_color() const noexcept;

protected:
    void rebuild_layout() override;
    void rebuild_focus_registry() override;

private:
    void create_internal_hosts();
    void clear_internal_host_pointers() noexcept;
    void collect_controls_from(const UiElement& element,std::vector<UiControl*>& out_controls,bool recurse_into_nested_scopes = true) const;
    [[nodiscard]] UiFocusScope* delegated_body_scope() noexcept;
    [[nodiscard]] const UiFocusScope* delegated_body_scope() const noexcept;
    [[nodiscard]] UiControl* first_header_focusable() const noexcept;
    [[nodiscard]] bool header_has_focusable_target() const noexcept;
    [[nodiscard]] bool enter_body_scope(bool focus_first_available);
    [[nodiscard]] bool leave_body_scope();
    void sync_body_scope_focus() noexcept;
    [[nodiscard]] bool event_targets_header(const UiInputEvent& event) const noexcept;
    [[nodiscard]] bool event_targets_body_scope(const UiInputEvent& event) const noexcept;
    [[nodiscard]] elysia::core::Rect header_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect body_rect() const noexcept;

private:
    UiListContainer* _left_actions = nullptr;
    SlotHost* _title_slot = nullptr;
    UiListContainer* _right_actions = nullptr;
    SlotHost* _body = nullptr;
    UiChromeContainerStyle _style{};
    UiLayoutPadding _header_padding{};
    UiLayoutPadding _body_padding{};
    float _header_height = 48.0f;
    bool _header_visible = true;
    bool _scope_focused = false;
    bool _body_scope_active = false;
};
}
