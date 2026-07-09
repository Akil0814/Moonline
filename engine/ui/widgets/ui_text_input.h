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

// Visual styling for text-input chrome, text, placeholder, and caret.
struct UiTextInputStyle
{
    UiChromeStyle chrome{};
    UiEnabledDisabledColors text{};
    UiEnabledDisabledColors placeholder{};
    elysia::core::Color caret{};
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

    // Replaces the current text buffer and updates caret/composition state as needed.
    void set_text(std::string text);
    [[nodiscard]] const std::string& text() const noexcept;
    void clear_text();

    // Sets placeholder content shown only when the text buffer is empty.
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
    // Returns true only when the text input should react to typing or pointer focus.
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    // Clears any pressed state left behind by focus loss or input cancellation.
    void clear_pushed_state() noexcept;
    // Drops IME composition state that should not survive focus or text resets.
    void clear_composition() noexcept;
    // Replaces the backing text buffer while controlling callbacks and caret repositioning.
    [[nodiscard]] bool set_text_internal(std::string text,bool notify_text_changed,bool move_caret_to_end);
    // Inserts new text at the current caret and advances the caret by inserted codepoints.
    [[nodiscard]] bool insert_text_at_caret(std::string_view text);
    // Removes the codepoint before the caret while preserving UTF-8 boundaries.
    [[nodiscard]] bool erase_previous_codepoint();
    // Removes the codepoint after the caret while preserving UTF-8 boundaries.
    [[nodiscard]] bool erase_next_codepoint();
    // Maps an x-coordinate in local space back to the nearest caret codepoint index.
    [[nodiscard]] std::size_t codepoint_index_at_x(int mouse_x) const;
    void move_caret_left() noexcept;
    void move_caret_right() noexcept;
    void move_caret_home() noexcept;
    void move_caret_end() noexcept;
    // Returns the padded interior used for text layout, caret placement, and hit testing.
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    // Measures text, placeholder, composition, and caret placement for rendering.
    [[nodiscard]] TextLayout compute_text_layout() const;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_border_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_text_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_placeholder_color() const noexcept;
    // Claims global text-input ownership so IME/text events route to this control.
    void acquire_text_input_ownership() const;
    // Releases global text-input ownership when the control no longer accepts typing.
    void release_text_input_ownership() const;
    // Emits text-changed only when the visible text buffer actually changed.
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


