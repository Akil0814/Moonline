#pragma once

#include "../containers/ui_panel.h"
#include "../containers/ui_scroll_container.h"
#include "../core/ui_control.h"
#include "../style/ui_style.h"
#include "../style/ui_theme_roles.h"
#include "../style/ui_visual_styles.h"
#include "../text/ui_text_content.h"
#include "../window/ui_transient_popup.h"
#include "ui_button.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

namespace elysia::ui
{
class UiListContainer;
class UiWindow;

// One popup row; disabled entries remain visible but are skipped by focus navigation.
struct UiDropdownOption
{
    UiTextContent content{};
    bool enabled = true;
};

using UiDropdownButtonSetSelectionChangedCallback = std::function<void(std::size_t selected_index)>;

// Trigger control plus a window-rendered transient popup kept outside ordinary child z-order.
class UiDropdownButtonSet final : public UiControl, public UiTransientPopup
{
public:
    explicit UiDropdownButtonSet(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiDropdownButtonSet(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiDropdownButtonSet(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiDropdownButtonSet() override = default;

    void reset() noexcept override;
    void set_enabled(bool enabled) override;
    void set_focused(bool focused) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_options(std::vector<UiDropdownOption> options);
    [[nodiscard]] const std::vector<UiDropdownOption>& options() const noexcept;
    void add_option(UiDropdownOption option);
    void clear_options();

    [[nodiscard]] std::optional<std::size_t> selected_index() const noexcept;
    [[nodiscard]] bool set_selected_index(std::size_t index);
    void set_on_selection_changed(UiDropdownButtonSetSelectionChangedCallback on_selection_changed);

    void open();
    void close() noexcept;
    void toggle();
    [[nodiscard]] bool is_expanded() const noexcept;

    // Registration lets the window prioritize popup rendering and input without taking ownership.
    void register_as_transient_popup(UiWindow& window);
    void unregister_as_transient_popup() noexcept;

    void set_style(const UiDropdownButtonSetStyle& style) noexcept;
    [[nodiscard]] const UiDropdownButtonSetStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept;
    void clear_style_override() noexcept;
    void set_theme_role(UiDropdownButtonSetThemeRole role) noexcept;
    [[nodiscard]] UiDropdownButtonSetThemeRole theme_role() const noexcept;

    [[nodiscard]] UiElement& transient_popup_owner() noexcept override;
    [[nodiscard]] const UiElement& transient_popup_owner() const noexcept override;
    [[nodiscard]] bool is_transient_popup_open() const noexcept override;
    [[nodiscard]] bool contains_transient_popup_point(int mouse_x,int mouse_y) const noexcept override;
    void close_transient_popup() noexcept override;
    bool on_transient_popup_input_event(const UiInputEvent& event) override;
    void submit_transient_popup_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

private:
    // Structural option changes rebuild rows; selection changes reuse the existing buttons.
    void create_popup_content();
    void rebuild_option_buttons();
    void sync_visual_state();
    void sync_popup_layout();
    void sync_theme_to_children(const UiTheme* theme = nullptr);
    [[nodiscard]] std::optional<std::size_t> first_enabled_option() const noexcept;
    [[nodiscard]] std::optional<std::size_t> next_enabled_option(int direction) const noexcept;
    void set_focused_option(std::optional<std::size_t> index) noexcept;
    void ensure_focused_option_visible() noexcept;
    [[nodiscard]] UiButton* option_button_at(std::size_t index) noexcept;
    [[nodiscard]] const UiButton* option_button_at(std::size_t index) const noexcept;
    [[nodiscard]] bool contains_trigger_point(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_pointer_event(const UiInputEvent& event) const noexcept;
    void apply_theme(const UiTheme& theme) override;

private:
    UiButton _trigger;
    UiPanel _popup_panel;
    UiScrollContainer _popup_scroll;
    UiListContainer* _popup_list = nullptr;
    std::vector<UiDropdownOption> _options;
    UiDropdownButtonSetSelectionChangedCallback _on_selection_changed;
    UiStyleState<UiDropdownButtonSetStyle> _style_state;
    UiDropdownButtonSetThemeRole _theme_role = UiDropdownButtonSetThemeRole::Default;
    UiWindow* _window = nullptr;
    std::optional<std::size_t> _selected_index;
    std::optional<std::size_t> _focused_option;
    bool _expanded = false;
};
}
