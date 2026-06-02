#include "ui_button.h"

#include "../ui_mouse_utils.h"
#include "../style/ui_theme.h"
#include "../style/ui_style.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

UiButton::UiButton(Vector2 position, Vector2 size, int order)
    : UiControl(position, size, order)
{
}

UiButton::UiButton(Vector2 position, Vector2 size, SDL_Texture* texture_message,
    Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up, int order)
    : UiButton(position, size, order)
{
    _texture_message = texture_message;
    _sound_effect_down = sound_effect_down;
    _sound_effect_up = sound_effect_up;
}

UiButton::UiButton(Vector2 position, Vector2 size, SDL_Texture* texture_message,
    Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
    SDL_Color color_idle, SDL_Color color_hovered, SDL_Color color_pushed, SDL_Color color_frame, int order)
    : UiButton(position, size, texture_message, sound_effect_down, sound_effect_up, order)
{
    set_local_state_colors(color_idle, color_hovered, color_pushed, color_frame);
}

UiButton::UiButton(Vector2 position, Vector2 size, SDL_Texture* texture_message,
    Mix_Chunk* sound_effect_down, Mix_Chunk* sound_effect_up,
    SDL_Texture* texture_idle, SDL_Texture* texture_hovered, SDL_Texture* texture_pushed, int order)
    : UiButton(position, size, texture_message, sound_effect_down, sound_effect_up, order)
{
    init_assert(texture_idle, "UiButton texture_idle must not be null.");
    init_assert(texture_hovered, "UiButton texture_hovered must not be null.");
    init_assert(texture_pushed, "UiButton texture_pushed must not be null.");

    _has_state_textures = true;
    _texture_idle = texture_idle;
    _texture_hovered = texture_hovered;
    _texture_pushed = texture_pushed;
    _appearance_mode = AppearanceMode::LocalTextures;
}

void UiButton::init_assert(const void* ptr, const char* err_msg) const
{
    if (ptr == nullptr)
    {
        throw std::invalid_argument(err_msg);
    }
}

void UiButton::on_render(SDL_Renderer* renderer)
{
    refresh_theme_if_needed();

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

    const Status draw_status = visual_status();
    if (!_has_state_textures)
    {
        switch (draw_status)
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

        switch (draw_status)
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

void UiButton::on_input(const InputSnapshot& input)
{
    if (!_enabled)
    {
        _is_pressing = false;
        _was_mouse_down = false;
        _status = Status::Idle;
        return;
    }

    if (!ui_mouse_input_allowed(input))
    {
        _is_pressing = false;
        _was_mouse_down = false;
        _status = Status::Idle;
        return;
    }

    const UiMouseState mouse_state = ui_current_mouse_state();
    const int mouse_x = mouse_state._position.x;
    const int mouse_y = mouse_state._position.y;

    if (ui_left_mouse_pressed(mouse_state, _was_mouse_down))
    {
        if (contains_point(mouse_x, mouse_y))
        {
            begin_press();
        }
    }

    if (ui_left_mouse_released(mouse_state, _was_mouse_down) && _is_pressing)
    {
        finish_press(mouse_x, mouse_y);
    }
    else if (!_is_pressing)
    {
        update_hover_status(mouse_x, mouse_y);
    }

    _was_mouse_down = mouse_state._left_button_down;
}

void UiButton::reset()
{
    UiControl::reset();
    _status = Status::Idle;
    _is_pressing = false;
    _was_mouse_down = false;
    _click_count = 0;
    _appearance_mode = AppearanceMode::Theme;
    mark_theme_dirty();
}

SDL_Rect UiButton::message_rect() const
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

bool UiButton::contains_point(int x, int y) const
{
    return ui_contains_point(rect(), x, y);
}

bool UiButton::update_hover_status(int x, int y)
{
    const Status new_status = contains_point(x, y) ? Status::Hovered : Status::Idle;
    const bool changed = _status != new_status;
    _status = new_status;
    return changed || new_status == Status::Hovered;
}

void UiButton::begin_press()
{
    _is_pressing = true;
    _status = Status::Pushed;
    play_sound(_sound_effect_down);
}

void UiButton::finish_press(int x, int y)
{
    _is_pressing = false;
    play_sound(_sound_effect_up);

    const bool hovered = contains_point(x, y);
    if (hovered)
    {
        ++_click_count;
        on_activated();
        if (_on_click)
        {
            _on_click();
        }
    }

    _status = hovered ? Status::Hovered : Status::Idle;
}

void UiButton::play_sound(Mix_Chunk* sound_effect) const
{
    if (sound_effect != nullptr)
    {
        Mix_PlayChannel(-1, sound_effect, 0);
    }
}

void UiButton::set_on_click(std::function<void()> func)
{
    _on_click = std::move(func);
}

UiButton::Status UiButton::status() const
{
    return _status;
}

void UiButton::set_message_texture(SDL_Texture* new_texture_message)
{
    _texture_message = new_texture_message;
}

void UiButton::set_local_state_colors(
    SDL_Color color_idle,
    SDL_Color color_hovered,
    SDL_Color color_pushed,
    SDL_Color color_frame
)
{
    set_state_colors_internal(
        color_idle,
        color_hovered,
        color_pushed,
        color_frame,
        AppearanceMode::LocalColors
    );
}

void UiButton::set_local_state_textures(
    SDL_Texture* texture_idle,
    SDL_Texture* texture_hovered,
    SDL_Texture* texture_pushed
)
{
    set_state_textures_internal(
        texture_idle,
        texture_hovered,
        texture_pushed,
        AppearanceMode::LocalTextures
    );
}

void UiButton::set_themed_state_colors(
    SDL_Color color_idle,
    SDL_Color color_hovered,
    SDL_Color color_pushed,
    SDL_Color color_frame
)
{
    set_state_colors_internal(
        color_idle,
        color_hovered,
        color_pushed,
        color_frame,
        AppearanceMode::Theme
    );
}

void UiButton::set_themed_state_textures(
    SDL_Texture* texture_idle,
    SDL_Texture* texture_hovered,
    SDL_Texture* texture_pushed
)
{
    set_state_textures_internal(
        texture_idle,
        texture_hovered,
        texture_pushed,
        AppearanceMode::Theme
    );
}

void UiButton::clear_state_textures()
{
    _has_state_textures = false;
    _texture_idle = nullptr;
    _texture_hovered = nullptr;
    _texture_pushed = nullptr;
}

void UiButton::use_theme_appearance()
{
    _appearance_mode = AppearanceMode::Theme;
    mark_theme_dirty();
}

bool UiButton::uses_theme_appearance() const
{
    return _appearance_mode == AppearanceMode::Theme;
}

UiButton::AppearanceMode UiButton::appearance_mode() const
{
    return _appearance_mode;
}

void UiButton::set_enabled(bool enabled)
{
    UiControl::set_enabled(enabled);
    if (!_enabled)
    {
        _is_pressing = false;
        _was_mouse_down = false;
        _status = Status::Idle;
    }
}

bool UiButton::handle_focused_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused)
    {
        return false;
    }

    if (event.type != InputEventType::Pressed || event.action != InputAction::Confirm)
    {
        return false;
    }

    play_sound(_sound_effect_down);
    play_sound(_sound_effect_up);
    ++_click_count;
    on_activated();
    if (_on_click)
    {
        _on_click();
    }

    return true;
}

int UiButton::click_count() const
{
    return _click_count;
}

void UiButton::reset_click_count()
{
    _click_count = 0;
}

void UiButton::set_button_theme_role(UiButtonThemeRole button_theme_role)
{
    _button_theme_role = button_theme_role;
    mark_theme_dirty();
}

UiButtonThemeRole UiButton::button_theme_role() const
{
    return _button_theme_role;
}

void UiButton::on_activated()
{
}

UiButton::Status UiButton::visual_status() const
{
    return _is_focused && _status == Status::Idle
        ? Status::Hovered
        : _status;
}

void UiButton::apply_theme(const UiTheme& theme)
{
    if (!uses_theme_appearance())
    {
        return;
    }

    const ButtonStyle* style = &theme._default_button;
    switch (_button_theme_role)
    {
    case UiButtonThemeRole::Default:
        style = &theme._default_button;
        break;

    case UiButtonThemeRole::Primary:
        style = &theme._primary_button;
        break;

    case UiButtonThemeRole::Danger:
        style = &theme._danger_button;
        break;
    }

    UiStyle::apply_button(*this, *style);
}

void UiButton::set_state_colors_internal(
    SDL_Color color_idle,
    SDL_Color color_hovered,
    SDL_Color color_pushed,
    SDL_Color color_frame,
    AppearanceMode appearance_mode
)
{
    _color_idle = color_idle;
    _color_hovered = color_hovered;
    _color_pushed = color_pushed;
    _color_frame = color_frame;
    _has_state_textures = false;
    _appearance_mode = appearance_mode;
}

void UiButton::set_state_textures_internal(
    SDL_Texture* texture_idle,
    SDL_Texture* texture_hovered,
    SDL_Texture* texture_pushed,
    AppearanceMode appearance_mode
)
{
    init_assert(texture_idle, "UiButton texture_idle must not be null.");
    init_assert(texture_hovered, "UiButton texture_hovered must not be null.");
    init_assert(texture_pushed, "UiButton texture_pushed must not be null.");

    _texture_idle = texture_idle;
    _texture_hovered = texture_hovered;
    _texture_pushed = texture_pushed;
    _has_state_textures = true;
    _appearance_mode = appearance_mode;
}
