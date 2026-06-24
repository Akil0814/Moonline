#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_control.h"

#include <functional>
#include <string>
#include <vector>

struct SDL_Texture;

namespace elysia::ui
{
enum class UiButtonVisualMode
{
    Text,
    Textured
};

struct UiButtonSounds
{
    std::string focus;
    std::string press;
    std::string click;
};

struct UiButtonTextures
{
    SDL_Texture* idle = nullptr;
    SDL_Texture* focused = nullptr;
    SDL_Texture* pushed = nullptr;
    SDL_Texture* disabled = nullptr;
};

class UiButton : public UiControl
{
public:
    using ClickCallback = std::function<void()>;

    explicit UiButton(
        const elysia::core::Rect& rect = elysia::core::Rect::zero(),
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    UiButton(
        const elysia::core::Vector2& position,
        const elysia::core::Vector2& size,
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    UiButton(
        const elysia::core::Vector2& center,
        const elysia::core::Vector2& size,
        UiFromCenterTag,
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    UiButton(
        const UiButtonTextures& textures,
        const elysia::core::Rect& rect,
        int order = 0
    ) noexcept;
    UiButton(
        const UiButtonTextures& textures,
        const elysia::core::Vector2& position,
        const elysia::core::Vector2& size,
        int order = 0
    ) noexcept;
    UiButton(
        const UiButtonTextures& textures,
        const elysia::core::Vector2& center,
        const elysia::core::Vector2& size,
        UiFromCenterTag,
        int order = 0
    ) noexcept;
    UiButton(
        SDL_Texture* idle,
        SDL_Texture* focused,
        SDL_Texture* pushed,
        SDL_Texture* disabled,
        const elysia::core::Rect& rect,
        int order = 0
    ) noexcept;
    UiButton(
        SDL_Texture* idle,
        SDL_Texture* focused,
        SDL_Texture* pushed,
        SDL_Texture* disabled,
        const elysia::core::Vector2& position,
        const elysia::core::Vector2& size,
        int order = 0
    ) noexcept;
    UiButton(
        SDL_Texture* idle,
        SDL_Texture* focused,
        SDL_Texture* pushed,
        SDL_Texture* disabled,
        const elysia::core::Vector2& center,
        const elysia::core::Vector2& size,
        UiFromCenterTag,
        int order = 0
    ) noexcept;
    ~UiButton() override = default;

    void reset() noexcept override;

    void set_enabled(bool enabled);
    void set_focused(bool focused);

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_state_textures(const UiButtonTextures& textures);
    void clear_state_textures();
    [[nodiscard]] bool has_state_textures() const noexcept;
    [[nodiscard]] UiButtonVisualMode visual_mode() const noexcept;

    void set_sounds(const UiButtonSounds& sounds);
    void clear_sounds();
    [[nodiscard]] const UiButtonSounds& sounds() const noexcept;

    void set_on_click(ClickCallback on_click);

    void set_idle_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color idle_color() const noexcept;

    void set_focused_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color focused_color() const noexcept;

    void set_pushed_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color pushed_color() const noexcept;

    void set_border_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color border_color() const noexcept;

    void set_text_color(elysia::core::Color color);
    [[nodiscard]] elysia::core::Color text_color() const noexcept;

    void set_text_point_size(int point_size);
    [[nodiscard]] int text_point_size() const noexcept;

    void set_padding(int padding);
    [[nodiscard]] int padding() const noexcept;

    void set_draw_border(bool draw_border);
    [[nodiscard]] bool draws_border() const noexcept;

private:
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] SDL_Texture* current_state_texture() const noexcept;
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect text_render_rect(SDL_Texture* text_texture) const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x, int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    void clear_pushed_state() noexcept;
    void play_sound_if_set(std::string_view sound_key) const;

private:
    std::string _text_key;
    UiButtonSounds _sounds;
    UiButtonTextures _state_textures;
    ClickCallback _on_click;
    UiButtonVisualMode _visual_mode = UiButtonVisualMode::Text;

    elysia::core::Color _idle_color = elysia::core::colors::cobalt_blue;
    elysia::core::Color _focused_color = elysia::core::colors::royal_blue;
    elysia::core::Color _pushed_color = elysia::core::colors::midnight_blue;
    elysia::core::Color _border_color = elysia::core::colors::sky_blue;
    elysia::core::Color _text_color = elysia::core::colors::white;

    int _text_point_size = 24;
    int _padding = 10;
    bool _draw_border = true;
    bool _is_pushed = false;
};

}