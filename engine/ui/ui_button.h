#pragma once
#include "style/ui_theme_roles.h"
#include "ui_control.h"

struct UiTheme;

#include <SDL.h>
#include <SDL_mixer.h>
#include <functional>

class UiButton : public UiControl
{
public:
    enum class Status
    {
        Idle = 0,
        Hovered,
        Pushed
    };

public:
    explicit UiButton(Vector2 position, Vector2 size, int order = 0);
    UiButton(Vector2 position, Vector2 size, SDL_Texture* texture_message,
        Mix_Chunk* sound_effect_down = nullptr, Mix_Chunk* sound_effect_up = nullptr, int order = 0);
    UiButton(Vector2 position, Vector2 size, SDL_Texture* texture_message,
        Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
        SDL_Color color_idle, SDL_Color color_hovered, SDL_Color color_pushed, SDL_Color color_frame, int order = 0);
    UiButton(Vector2 position, Vector2 size, SDL_Texture* texture_message,
        Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
        SDL_Texture* texture_idle, SDL_Texture* texture_hovered, SDL_Texture* texture_pushed, int order = 0);

    UiButton(const UiButton&) = delete;
    UiButton& operator=(const UiButton&) = delete;
    ~UiButton() = default;

    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_message_texture(SDL_Texture* new_texture_message);
    void set_state_colors(
        SDL_Color color_idle,
        SDL_Color color_hovered,
        SDL_Color color_pushed,
        SDL_Color color_frame
    );
    void set_state_textures(
        SDL_Texture* texture_idle,
        SDL_Texture* texture_hovered,
        SDL_Texture* texture_pushed
    );
    void clear_state_textures();
    void set_enabled(bool enabled) override;
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;
    [[nodiscard]] int click_count() const;
    void reset_click_count();
    void set_on_click(std::function<void()> func);
    [[nodiscard]] Status status() const;
    [[nodiscard]] Status get_status() const;
    void set_button_theme_role(UiButtonThemeRole button_theme_role);
    [[nodiscard]] UiButtonThemeRole button_theme_role() const;

private:
    [[nodiscard]] SDL_Rect message_rect() const;
    [[nodiscard]] bool contains_point(int x, int y) const;
    bool update_hover_status(int x, int y);
    void begin_press();
    void finish_press(int x, int y);
    void play_sound(Mix_Chunk* sound_effect) const;
    void init_assert(const void* ptr, const char* err_msg) const;
    void apply_theme(const UiTheme& theme) override;

private:
    Mix_Chunk* _sound_effect_down = nullptr;
    Mix_Chunk* _sound_effect_up = nullptr;

    SDL_Texture* _texture_message = nullptr;
    SDL_Texture* _texture_idle = nullptr;
    SDL_Texture* _texture_hovered = nullptr;
    SDL_Texture* _texture_pushed = nullptr;

    SDL_Color _color_frame{ 0, 0, 0, 255 };
    SDL_Color _color_idle{ 180, 180, 180, 255 };
    SDL_Color _color_hovered{ 200, 200, 200, 255 };
    SDL_Color _color_pushed{ 130, 130, 130, 255 };

    Status _status = Status::Idle;
    std::function<void()> _on_click;

    bool _has_state_textures = false;
    bool _is_pressing = false;
    bool _was_mouse_down = false;

    int _click_count = 0;
    UiButtonThemeRole _button_theme_role = UiButtonThemeRole::Default;
};
