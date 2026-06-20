#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_control.h"

#include <functional>
#include <string>
#include <vector>

struct SDL_Texture;
struct UiRenderCommand;

struct UiButtonTextures
{
    SDL_Texture* idle;
    SDL_Texture*  hovered;
    SDL_Texture*  pressed;
    SDL_Texture* disabled;
};

class UiButton : public UiControl
{
public:
    using ClickCallback = std::function<void()>;

    explicit UiButton(
        const Rect& rect = Rect::zero(),
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    UiButton(
        const Vector2& position,
        const Vector2& size,
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    ~UiButton() override = default;

    void reset() noexcept override;

    void set_enabled(bool enabled);
    void set_focused(bool focused);

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const override;

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_on_click(ClickCallback on_click);

    void set_idle_color(Color color);
    [[nodiscard]] Color idle_color() const noexcept;

    void set_focused_color(Color color);
    [[nodiscard]] Color focused_color() const noexcept;

    void set_pushed_color(Color color);
    [[nodiscard]] Color pushed_color() const noexcept;

    void set_border_color(Color color);
    [[nodiscard]] Color border_color() const noexcept;

    void set_text_color(Color color);
    [[nodiscard]] Color text_color() const noexcept;

    void set_text_point_size(int point_size);
    [[nodiscard]] int text_point_size() const noexcept;

    void set_padding(int padding);
    [[nodiscard]] int padding() const noexcept;

    void set_draw_border(bool draw_border);
    [[nodiscard]] bool draws_border() const noexcept;

private:
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] Color current_background_color() const noexcept;
    [[nodiscard]] Rect content_rect() const noexcept;
    [[nodiscard]] Rect text_render_rect(SDL_Texture* text_texture) const noexcept;
    void clear_pushed_state() noexcept;

private:
    std::string _text_key;
    ClickCallback _on_click;

    Color _idle_color = colors::loading_blue_button_idle;
    Color _focused_color = colors::loading_blue_button_hovered;
    Color _pushed_color = colors::loading_blue_button_pushed;
    Color _border_color = colors::loading_blue_button_frame;
    Color _text_color = colors::white;

    int _text_point_size = 24;
    int _padding = 10;
    bool _draw_border = true;
    bool _is_pushed = false;
};
