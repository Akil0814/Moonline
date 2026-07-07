#pragma once

#include "../../core/render/colors.h"
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

class UiTextInput : public UiControl
{
public:
    explicit UiTextInput(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiTextInput(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiTextInput(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiTextInput() override = default;

    void reset() noexcept override;

    void set_enabled(bool enabled);
    void set_focused(bool focused);

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

    void set_idle_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color idle_color() const noexcept;
    void set_focused_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color focused_color() const noexcept;
    void set_pushed_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color pushed_color() const noexcept;
    void set_disabled_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_background_color() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;
    void set_disabled_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_border_color() const noexcept;
    void set_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color text_color() const noexcept;
    void set_disabled_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_text_color() const noexcept;
    void set_placeholder_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color placeholder_color() const noexcept;
    void set_disabled_placeholder_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_placeholder_color() const noexcept;
    void set_caret_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color caret_color() const noexcept;

    void set_text_point_size(int point_size) noexcept;
    [[nodiscard]] int text_point_size() const noexcept;
    void set_padding(int padding) noexcept;
    [[nodiscard]] int padding() const noexcept;
    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;

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
    elysia::core::Color _idle_color = elysia::core::colors::cobalt_blue;
    elysia::core::Color _focused_color = elysia::core::colors::royal_blue;
    elysia::core::Color _pushed_color = elysia::core::colors::midnight_blue;
    elysia::core::Color _disabled_background_color = elysia::core::colors::gray_700;
    elysia::core::Color _border_color = elysia::core::colors::sky_blue;
    elysia::core::Color _disabled_border_color = elysia::core::colors::gray_500;
    elysia::core::Color _text_color = elysia::core::colors::white;
    elysia::core::Color _disabled_text_color = elysia::core::colors::gray_300;
    elysia::core::Color _placeholder_color = elysia::core::colors::gray_300;
    elysia::core::Color _disabled_placeholder_color = elysia::core::colors::gray_500;
    elysia::core::Color _caret_color = elysia::core::colors::glacial_white;
    int _text_point_size = 24;
    int _padding = 10;
    bool _draw_background = true;
    bool _draw_border = true;
    bool _is_pushed = false;
};
}
