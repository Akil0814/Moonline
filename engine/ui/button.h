#pragma once
#include "../core/game_object.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <functional>

class Button : public GameObject
{
public:
    enum class Status
    {
        Idle = 0,
        Hovered,
        Pushed
    };

public:
    explicit Button(Vector2 position, Vector2 size, int order = 0);
    Button(Vector2 position, Vector2 size, SDL_Texture* texture_message,
        Mix_Chunk* sound_effect_down = nullptr, Mix_Chunk* sound_effect_up = nullptr, int order = 0);
    Button(Vector2 position, Vector2 size, SDL_Texture* texture_message,
        Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
        SDL_Color color_idle, SDL_Color color_hovered, SDL_Color color_pushed, SDL_Color color_frame, int order = 0);
    Button(Vector2 position, Vector2 size, SDL_Texture* texture_message,
        Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
        SDL_Texture* texture_idle, SDL_Texture* texture_hovered, SDL_Texture* texture_pushed, int order = 0);

    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    Button(Button&&) = default;
    Button& operator=(Button&&) = default;
    ~Button() = default;

    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_message_texture(SDL_Texture* new_texture_message);
    void set_enabled(bool enabled);
    [[nodiscard]] bool is_enabled() const;
    [[nodiscard]] int click_count() const;
    void reset_click_count();
    void set_on_click(std::function<void()> func);
    [[nodiscard]] Status status() const;
    [[nodiscard]] Status get_status() const;

private:
    [[nodiscard]] SDL_Rect message_rect() const;
    [[nodiscard]] bool contains_point(int x, int y) const;
    bool update_hover_status(int x, int y);
    void begin_press();
    void finish_press(int x, int y);
    void play_sound(Mix_Chunk* sound_effect) const;
    void init_assert(const void* ptr, const char* err_msg) const;

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
    bool _enabled = true;
    bool _is_pressing = false;
    bool _was_mouse_down = false;

    int _click_count = 0;
};
