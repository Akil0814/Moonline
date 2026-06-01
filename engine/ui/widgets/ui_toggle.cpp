#include "ui_toggle.h"

#include <SDL.h>

UiToggle::UiToggle(Vector2 position, Vector2 size, int order)
    : GameObject(DepthLayer::UI, order),
      _label(Vector2::zero(), Vector2::zero(), order + 1),
      _value_label(Vector2::zero(), Vector2::zero(), order + 1)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
    reset();
}

void UiToggle::on_update(double delta)
{
    _label.on_update(delta);
    _value_label.on_update(delta);
}

void UiToggle::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    const SDL_Rect toggle_rect = rect();
    if (toggle_rect.w <= 0 || toggle_rect.h <= 0)
    {
        return;
    }

    Uint8 old_r = 0;
    Uint8 old_g = 0;
    Uint8 old_b = 0;
    Uint8 old_a = 0;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);

    const SDL_Color fill_color = _is_focused ? _focused_background_color : _background_color;
    SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    SDL_RenderFillRect(renderer, &toggle_rect);
    SDL_SetRenderDrawColor(renderer, _border_color.r, _border_color.g, _border_color.b, _border_color.a);
    SDL_RenderDrawRect(renderer, &toggle_rect);

    const SDL_Rect indicator_rect{
        toggle_rect.x + 10,
        toggle_rect.y + 10,
        std::max(12, toggle_rect.h - 20),
        std::max(12, toggle_rect.h - 20)
    };
    SDL_SetRenderDrawColor(
        renderer,
        _value ? 180 : 52,
        _value ? 214 : 82,
        _value ? 246 : 118,
        255
    );
    SDL_RenderFillRect(renderer, &indicator_rect);
    SDL_SetRenderDrawColor(renderer, 18, 22, 28, 255);
    SDL_RenderDrawRect(renderer, &indicator_rect);
    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);

    _label.on_render(renderer);
    _value_label.on_render(renderer);
}

void UiToggle::on_input(const InputSnapshot& input)
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

    if (!mouse_down && _was_mouse_down && contains_point(mouse_x, mouse_y))
    {
        toggle();
    }

    _was_mouse_down = mouse_down;
}

void UiToggle::reset()
{
    GameObject::reset();
    _label.reset();
    _label.set_font_key(_font_key);
    _label.set_auto_size(false);
    _label.set_horizontal_align(TextHorizontalAlign::Left);
    _label.set_vertical_align(TextVerticalAlign::Center);
    _label.set_text_color(_text_color);

    _value_label.reset();
    _value_label.set_font_key(_font_key);
    _value_label.set_auto_size(false);
    _value_label.set_horizontal_align(TextHorizontalAlign::Right);
    _value_label.set_vertical_align(TextVerticalAlign::Center);
    _value_label.set_text_color(_text_color);

    _on_changed = nullptr;
    _label_text.clear();
    _off_text = "Off";
    _on_text = "On";
    _font_key = "ui.default";
    _text_color = SDL_Color{ 244, 244, 248, 255 };
    _background_color = SDL_Color{ 32, 42, 58, 220 };
    _focused_background_color = SDL_Color{ 52, 78, 116, 235 };
    _border_color = SDL_Color{ 112, 140, 180, 255 };
    _value = false;
    _enabled = true;
    _is_focused = false;
    _was_mouse_down = false;

    sync_labels();
}

void UiToggle::set_value(bool value)
{
    set_value_internal(value, true);
}

bool UiToggle::value() const
{
    return _value;
}

void UiToggle::set_state_texts(const std::string& off_text, const std::string& on_text)
{
    _off_text = off_text;
    _on_text = on_text;
    sync_labels();
}

const std::string& UiToggle::off_text() const
{
    return _off_text;
}

const std::string& UiToggle::on_text() const
{
    return _on_text;
}

const std::string& UiToggle::display_text() const
{
    return _value ? _on_text : _off_text;
}

void UiToggle::set_label_text(const std::string& label_text)
{
    _label_text = label_text;
    sync_labels();
}

const std::string& UiToggle::label_text() const
{
    return _label_text;
}

void UiToggle::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    _label.set_font_key(font_key);
    _value_label.set_font_key(font_key);
}

const std::string& UiToggle::font_key() const
{
    return _font_key;
}

void UiToggle::set_text_color(SDL_Color color)
{
    _text_color = color;
    _label.set_text_color(color);
    _value_label.set_text_color(color);
}

SDL_Color UiToggle::text_color() const
{
    return _text_color;
}

void UiToggle::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color UiToggle::background_color() const
{
    return _background_color;
}

void UiToggle::set_focused_background_color(SDL_Color color)
{
    _focused_background_color = color;
}

SDL_Color UiToggle::focused_background_color() const
{
    return _focused_background_color;
}

void UiToggle::set_border_color(SDL_Color color)
{
    _border_color = color;
}

SDL_Color UiToggle::border_color() const
{
    return _border_color;
}

void UiToggle::set_enabled(bool enabled)
{
    _enabled = enabled;
}

bool UiToggle::is_enabled() const
{
    return _enabled;
}

void UiToggle::set_focused(bool focused)
{
    _is_focused = focused;
}

bool UiToggle::is_focused() const
{
    return _is_focused;
}

void UiToggle::set_on_changed(UiToggleChangedCallback on_changed)
{
    _on_changed = std::move(on_changed);
}

bool UiToggle::handle_focused_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused || event.type != InputEventType::Pressed)
    {
        return false;
    }

    switch (event.action)
    {
    case InputAction::Confirm:
        toggle();
        return true;

    case InputAction::Left:
        set_value_internal(false, true);
        return true;

    case InputAction::Right:
        set_value_internal(true, true);
        return true;

    default:
        return false;
    }
}

GameObject* UiToggle::game_object()
{
    return this;
}

const GameObject* UiToggle::game_object() const
{
    return this;
}

void UiToggle::toggle()
{
    set_value_internal(!_value, true);
}

void UiToggle::set_value_internal(bool value, bool notify)
{
    if (_value == value)
    {
        return;
    }

    _value = value;
    sync_labels();

    if (notify && _on_changed)
    {
        _on_changed(_value);
    }
}

void UiToggle::sync_labels()
{
    const SDL_Rect toggle_rect = rect();
    _label.set_font_key(_font_key);
    _label.set_text_color(_text_color);
    _label.set_world_position({
        position().x + 18.0f + static_cast<float>(std::max(12, toggle_rect.h - 20)),
        position().y
    });
    _label.set_size({
        std::max(0.0f, size().x * 0.46f),
        size().y
    });
    _label.set_text(_label_text);

    _value_label.set_font_key(_font_key);
    _value_label.set_text_color(_text_color);
    _value_label.set_world_position({
        position().x + size().x * 0.48f,
        position().y
    });
    _value_label.set_size({
        std::max(0.0f, size().x * 0.44f - 12.0f),
        size().y
    });
    _value_label.set_text(display_text());
}

bool UiToggle::contains_point(int x, int y) const
{
    const SDL_Point point{ x, y };
    return SDL_PointInRect(&point, &rect()) == SDL_TRUE;
}
