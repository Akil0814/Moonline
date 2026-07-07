#pragma once

#include "../../core/render/colors.h"
#include "../style/ui_interaction_style.h"
#include "../core/ui_control.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace elysia::ui
{
using UiTextInputChangedCallback = std::function<void(std::string_view text)>;
using UiTextInputSubmitCallback = std::function<void(std::string_view text)>;

struct UiTextInputStyle
{
    UiChromeStyle chrome{};
    UiEnabledDisabledColors text{};
    UiEnabledDisabledColors placeholder{
        UiPalette::text_muted,
        UiPalette::text_disabled
    };
    elysia::core::Color caret = UiPalette::caret;
};

class UiTextInput : public UiControl
{
public:
    explicit UiTextInput(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiTextInput(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiTextInput(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiTextInput() override;

    void reset() noexcept override;

    void set_enabled(bool enabled) override;
    void set_focused(bool focused) override;

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_text(std::string text);
    [[nodiscard]] const std::string& text() const noexcept;
    void clear_text();

    void set_placeholder_text(std::string placeholder_text);
    [[nodiscard]] const std::string& placeholder_text() const noexcept;

    void set_on_text_changed(UiTextInputChangedCallback on_text_changed);
    void set_on_submit(UiTextInputSubmitCallback on_submit);

    void set_max_length(std::optional<std::size_t> max_length);
    [[nodiscard]] const std::optional<std::size_t>& max_length() const noexcept;

    void set_style(const UiTextInputStyle& style) noexcept;
    [[nodiscard]] const UiTextInputStyle& style() const noexcept;

    void set_text_point_size(int point_size) noexcept;
    [[nodiscard]] int text_point_size() const noexcept;
    void set_padding(int padding) noexcept;
    [[nodiscard]] int padding() const noexcept;

private:
    struct TextLayout;

private:
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    void clear_pushed_state() noexcept;
    void clear_composition() noexcept;
    [[nodiscard]] bool set_text_internal(std::string text,bool notify_text_changed,bool move_caret_to_end);
    [[nodiscard]] bool insert_text_at_caret(std::string_view text);
    [[nodiscard]] bool erase_previous_codepoint();
    [[nodiscard]] bool erase_next_codepoint();
    [[nodiscard]] std::size_t codepoint_index_at_x(int mouse_x) const;
    void move_caret_left() noexcept;
    void move_caret_right() noexcept;
    void move_caret_home() noexcept;
    void move_caret_end() noexcept;
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] TextLayout compute_text_layout() const;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_border_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_text_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_placeholder_color() const noexcept;
    void acquire_text_input_ownership() const;
    void release_text_input_ownership() const;
    void notify_text_changed_if_needed(const std::string& previous_text) const;

private:
    UiTextInputChangedCallback _on_text_changed;
    UiTextInputSubmitCallback _on_submit;
    std::string _text;
    std::string _placeholder_text;
    std::string _composition_text;
    std::size_t _caret_codepoint_index = 0;
    std::size_t _composition_insert_codepoint_index = 0;
    int _composition_start = 0;
    int _composition_length = 0;
    std::optional<std::size_t> _max_length = std::nullopt;
    UiTextInputStyle _style{};
    int _text_point_size = 24;
    int _padding = 10;
    bool _is_pushed = false;
};
}
