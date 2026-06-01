#include "button.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

Button::Button(Vector2 position, Vector2 size, int order)
    : GameObject(DepthLayer::UI, order)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
}

Button::Button(Vector2 position, Vector2 size, SDL_Texture* texture_message,
    Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up, int order)
    : Button(position, size, order)
{
    _texture_message = texture_message;
    _sound_effect_down = sound_effect_down;
    _sound_effect_up = sound_effect_up;
}

Button::Button(Vector2 position, Vector2 size, SDL_Texture* texture_message,
    Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
    SDL_Color color_idle, SDL_Color color_hovered, SDL_Color color_pushed, SDL_Color color_frame, int order)
    : Button(position, size, texture_message, sound_effect_down, sound_effect_up, order)
{
    _color_idle = color_idle;
    _color_hovered = color_hovered;
    _color_pushed = color_pushed;
    _color_frame = color_frame;
}

Button::Button(Vector2 position, Vector2 size, SDL_Texture* texture_message,
    Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
    SDL_Texture* texture_idle, SDL_Texture* texture_hovered, SDL_Texture* texture_pushed, int order)
    : Button(position, size, texture_message, sound_effect_down, sound_effect_up, order)
{
    init_assert(texture_idle, "Button texture_idle must not be null.");
    init_assert(texture_hovered, "Button texture_hovered must not be null.");
    init_assert(texture_pushed, "Button texture_pushed must not be null.");

    _has_state_textures = true;
    _texture_idle = texture_idle;
    _texture_hovered = texture_hovered;
    _texture_pushed = texture_pushed;
}

void Button::init_assert(const void* ptr, const char* err_msg) const
{
    if (ptr == nullptr)
    {
        throw std::invalid_argument(err_msg);
    }
}

void Button::on_render(SDL_Renderer* renderer)
{
    const SDL_Rect& button_rect = rect();
    if (!renderer || button_rect.w <= 0 || button_rect.h <= 0)
    {
        return;
    }

    Uint8 old_r = 0;
    Uint8 old_g = 0;
    Uint8 old_b = 0;
    Uint8 old_a = 0;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);

    if (!_has_state_textures)
    {
        switch (_status)
        {
        case Status::Idle:
            SDL_SetRenderDrawColor(renderer, _color_idle.r, _color_idle.g, _color_idle.b, _color_idle.a);
            break;
        case Status::Hovered:
            SDL_SetRenderDrawColor(renderer, _color_hovered.r, _color_hovered.g, _color_hovered.b, _color_hovered.a);
            break;
        case Status::Pushed:
            SDL_SetRenderDrawColor(renderer, _color_pushed.r, _color_pushed.g, _color_pushed.b, _color_pushed.a);
            break;
        }

        SDL_RenderFillRect(renderer, &button_rect);
        SDL_SetRenderDrawColor(renderer, _color_frame.r, _color_frame.g, _color_frame.b, _color_frame.a);
        SDL_RenderDrawRect(renderer, &button_rect);
    }
    else
    {
        SDL_Texture* current_texture = _texture_idle;
        switch (_status)
        {
        case Status::Idle:
            current_texture = _texture_idle;
            break;
        case Status::Hovered:
            current_texture = _texture_hovered;
            break;
        case Status::Pushed:
            current_texture = _texture_pushed;
            break;
        }

        SDL_RenderCopy(renderer, current_texture, nullptr, &button_rect);
    }

    if (_texture_message != nullptr)
    {
        SDL_Rect message_dst = message_rect();
        SDL_RenderCopy(renderer, _texture_message, nullptr, &message_dst);
    }

    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

void Button::on_input(const InputSnapshot& input)
{
    (void)input;

    if (!_enabled)
    {
        _was_mouse_down = false;
        return;
    }

    int mouse_x = 0;
    int mouse_y = 0;
    const Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    const bool mouse_down = (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    if (mouse_down && !_was_mouse_down)
    {
        if (contains_point(mouse_x, mouse_y))
        {
            begin_press();
        }
    }

    if (!mouse_down && _was_mouse_down && _is_pressing)
    {
        finish_press(mouse_x, mouse_y);
    }
    else if (!_is_pressing)
    {
        update_hover_status(mouse_x, mouse_y);
    }

    _was_mouse_down = mouse_down;
}

void Button::reset()
{
    GameObject::reset();
    _status = Status::Idle;
    _enabled = true;
    _is_pressing = false;
    _was_mouse_down = false;
    _click_count = 0;
}

SDL_Rect Button::message_rect() const
{
    const Vector2& button_pos = position();
    const Vector2& button_size = size();

    int texture_width = 0;
    int texture_height = 0;
    if (_texture_message != nullptr)
    {
        SDL_QueryTexture(_texture_message, nullptr, nullptr, &texture_width, &texture_height);
    }

    const int draw_width = std::max(0, texture_width);
    const int draw_height = std::max(0, texture_height);
    const int button_width = static_cast<int>(button_size.x);
    const int button_height = static_cast<int>(button_size.y);

    return {
        static_cast<int>(button_pos.x) + (button_width - draw_width) / 2,
        static_cast<int>(button_pos.y) + (button_height - draw_height) / 2,
        draw_width,
        draw_height
    };
}

bool Button::contains_point(int x, int y) const
{
    const SDL_Point cursor = { x, y };
    const SDL_Rect& button_rect = rect();
    return SDL_PointInRect(&cursor, &button_rect) == SDL_TRUE;
}

bool Button::update_hover_status(int x, int y)
{
    const Status new_status = contains_point(x, y) ? Status::Hovered : Status::Idle;
    const bool changed = _status != new_status;
    _status = new_status;
    return changed || new_status == Status::Hovered;
}

void Button::begin_press()
{
    _is_pressing = true;
    _status = Status::Pushed;
    play_sound(_sound_effect_down);
}

void Button::finish_press(int x, int y)
{
    _is_pressing = false;
    play_sound(_sound_effect_up);

    const bool hovered = contains_point(x, y);
    if (hovered)
    {
        ++_click_count;
        if (_on_click)
        {
            _on_click();
        }
    }

    _status = hovered ? Status::Hovered : Status::Idle;
}

void Button::play_sound(Mix_Chunk* sound_effect) const
{
    if (sound_effect != nullptr)
    {
        Mix_PlayChannel(-1, sound_effect, 0);
    }
}

void Button::set_on_click(std::function<void()> func)
{
    _on_click = std::move(func);
}

Button::Status Button::status() const
{
    return _status;
}

Button::Status Button::get_status() const
{
    return status();
}

void Button::set_message_texture(SDL_Texture* new_texture_message)
{
    _texture_message = new_texture_message;
}

void Button::set_enabled(bool new_enabled)
{
    _enabled = new_enabled;
    if (!_enabled)
    {
        _is_pressing = false;
        _was_mouse_down = false;
        _status = Status::Idle;
    }
}

bool Button::is_enabled() const
{
    return _enabled;
}

int Button::click_count() const
{
    return _click_count;
}

void Button::reset_click_count()
{
    _click_count = 0;
}
