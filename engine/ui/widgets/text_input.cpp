#include "text_input.h"

#include "../../resources/resource_manager.h"

#include <algorithm>

TextInput::TextInput(Vector2 position, Vector2 size, int order)
    : GameObject(DepthLayer::UI, order)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
    reset();
}

void TextInput::on_update(double delta)
{
    if (_is_focused)
    {
        _caret_timer += delta;
        if (_caret_timer >= 0.5)
        {
            _caret_timer = 0.0;
            _show_caret = !_show_caret;
        }
    }
    else
    {
        _show_caret = false;
        _caret_timer = 0.0;
    }
}

void TextInput::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    refresh_texture(renderer);

    const SDL_Rect input_rect = rect();
    Uint8 old_r = 0;
    Uint8 old_g = 0;
    Uint8 old_b = 0;
    Uint8 old_a = 0;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);

    SDL_SetRenderDrawColor(
        renderer,
        _background_color.r,
        _background_color.g,
        _background_color.b,
        _background_color.a
    );
    SDL_RenderFillRect(renderer, &input_rect);

    const SDL_Color current_border = _is_focused ? _focused_border_color : _border_color;
    SDL_SetRenderDrawColor(
        renderer,
        current_border.r,
        current_border.g,
        current_border.b,
        current_border.a
    );
    SDL_RenderDrawRect(renderer, &input_rect);

    if (_texture)
    {
        SDL_Rect text_rect{
            input_rect.x + _padding,
            input_rect.y + std::max(0, (input_rect.h - _texture_height) / 2),
            _texture_width,
            _texture_height
        };
        SDL_RenderCopy(renderer, _texture.get(), nullptr, &text_rect);
    }

    if (_is_focused && _show_caret)
    {
        const int caret_x = caret_pixel_x();
        SDL_SetRenderDrawColor(
            renderer,
            _caret_color.r,
            _caret_color.g,
            _caret_color.b,
            _caret_color.a
        );
        SDL_RenderDrawLine(
            renderer,
            caret_x,
            input_rect.y + _padding / 2,
            caret_x,
            input_rect.y + input_rect.h - _padding / 2
        );
    }

    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
}

void TextInput::on_input(const InputSnapshot& input)
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
        update_caret_from_mouse(mouse_x);
    }

    _was_mouse_down = mouse_down;
}

void TextInput::reset()
{
    GameObject::reset();
    _text.clear();
    _placeholder_text.clear();
    _font_key = "ui.default";
    _font = nullptr;
    _texture.reset();
    _on_text_changed = nullptr;
    _text_color = SDL_Color{ 244, 244, 248, 255 };
    _placeholder_color = SDL_Color{ 142, 154, 170, 255 };
    _background_color = SDL_Color{ 28, 36, 52, 220 };
    _border_color = SDL_Color{ 110, 140, 182, 255 };
    _focused_border_color = SDL_Color{ 166, 208, 255, 255 };
    _caret_color = SDL_Color{ 248, 248, 252, 255 };
    _max_length = 64;
    _caret_index = 0;
    _padding = 10;
    _texture_width = 0;
    _texture_height = 0;
    _caret_timer = 0.0;
    _read_only = false;
    _password_mode = false;
    _enabled = true;
    _is_focused = false;
    _show_caret = false;
    _dirty = true;
    _was_mouse_down = false;
}

void TextInput::set_text(const std::string& text)
{
    set_text_internal(text, true);
}

const std::string& TextInput::text() const
{
    return _text;
}

void TextInput::set_placeholder_text(const std::string& placeholder_text)
{
    _placeholder_text = placeholder_text;
    mark_dirty();
}

const std::string& TextInput::placeholder_text() const
{
    return _placeholder_text;
}

void TextInput::set_font(TTF_Font* font)
{
    _font = font;
    mark_dirty();
}

TTF_Font* TextInput::font() const
{
    return _font;
}

void TextInput::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    mark_dirty();
}

const std::string& TextInput::font_key() const
{
    return _font_key;
}

void TextInput::set_max_length(size_t max_length)
{
    _max_length = max_length;
    if (_text.size() > _max_length)
    {
        set_text_internal(_text.substr(0, _max_length), true);
    }
}

size_t TextInput::max_length() const
{
    return _max_length;
}

void TextInput::set_read_only(bool read_only)
{
    _read_only = read_only;
}

bool TextInput::is_read_only() const
{
    return _read_only;
}

void TextInput::set_password_mode(bool password_mode)
{
    _password_mode = password_mode;
    mark_dirty();
}

bool TextInput::is_password_mode() const
{
    return _password_mode;
}

void TextInput::set_enabled(bool enabled)
{
    _enabled = enabled;
    if (!_enabled)
    {
        set_focused(false);
    }
}

bool TextInput::is_enabled() const
{
    return _enabled;
}

void TextInput::set_focused(bool focused)
{
    if (_is_focused == focused)
    {
        return;
    }

    _is_focused = focused && _enabled;
    _show_caret = _is_focused;
    _caret_timer = 0.0;

    if (_is_focused)
    {
        SDL_StartTextInput();
    }
    else
    {
        SDL_StopTextInput();
    }
}

bool TextInput::is_focused() const
{
    return _is_focused;
}

void TextInput::set_padding(int padding)
{
    _padding = std::max(0, padding);
}

int TextInput::padding() const
{
    return _padding;
}

void TextInput::set_text_color(SDL_Color color)
{
    _text_color = color;
    mark_dirty();
}

SDL_Color TextInput::text_color() const
{
    return _text_color;
}

void TextInput::set_placeholder_color(SDL_Color color)
{
    _placeholder_color = color;
    mark_dirty();
}

SDL_Color TextInput::placeholder_color() const
{
    return _placeholder_color;
}

void TextInput::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color TextInput::background_color() const
{
    return _background_color;
}

void TextInput::set_border_color(SDL_Color color)
{
    _border_color = color;
}

SDL_Color TextInput::border_color() const
{
    return _border_color;
}

void TextInput::set_focused_border_color(SDL_Color color)
{
    _focused_border_color = color;
}

SDL_Color TextInput::focused_border_color() const
{
    return _focused_border_color;
}

void TextInput::set_caret_color(SDL_Color color)
{
    _caret_color = color;
}

SDL_Color TextInput::caret_color() const
{
    return _caret_color;
}

void TextInput::set_on_text_changed(TextInputChangedCallback on_text_changed)
{
    _on_text_changed = std::move(on_text_changed);
}

bool TextInput::handle_focused_input_event(const InputEvent& event)
{
    if (!_enabled || !_is_focused)
    {
        return false;
    }

    if (event.type == InputEventType::TextInput)
    {
        insert_text(event.text);
        return true;
    }

    if (event.type != InputEventType::Pressed)
    {
        return false;
    }

    switch (event.action)
    {
    case InputAction::Left:
        move_caret_left();
        return true;

    case InputAction::Right:
        move_caret_right();
        return true;

    case InputAction::Home:
        _caret_index = 0;
        _show_caret = true;
        _caret_timer = 0.0;
        return true;

    case InputAction::End:
        _caret_index = _text.size();
        _show_caret = true;
        _caret_timer = 0.0;
        return true;

    case InputAction::Backspace:
        erase_left();
        return true;

    case InputAction::DeleteKey:
        erase_right();
        return true;

    default:
        return false;
    }
}

GameObject* TextInput::game_object()
{
    return this;
}

const GameObject* TextInput::game_object() const
{
    return this;
}

void TextInput::mark_dirty()
{
    _dirty = true;
}

void TextInput::refresh_texture(SDL_Renderer* renderer)
{
    if (!_dirty)
    {
        return;
    }

    _dirty = false;
    _texture.reset();
    _texture_width = 0;
    _texture_height = 0;

    const std::string render_text = display_text().empty() ? _placeholder_text : display_text();
    if (render_text.empty())
    {
        return;
    }

    TTF_Font* resolved_font = resolve_font();
    if (!resolved_font)
    {
        return;
    }

    const SDL_Color render_color = display_text().empty() ? _placeholder_color : _text_color;
    SDL_Surface* text_surface = TTF_RenderUTF8_Blended(resolved_font, render_text.c_str(), render_color);
    if (!text_surface)
    {
        return;
    }

    SDL_Texture* raw_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    _texture_width = text_surface->w;
    _texture_height = text_surface->h;
    SDL_FreeSurface(text_surface);

    if (!raw_texture)
    {
        _texture_width = 0;
        _texture_height = 0;
        return;
    }

    _texture.reset(raw_texture);
    SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND);
}

TTF_Font* TextInput::resolve_font() const
{
    if (_font)
    {
        return _font;
    }

    if (_font_key.empty())
    {
        return nullptr;
    }

    return ResourceManager::instance()->find_font(_font_key);
}

std::string TextInput::display_text() const
{
    if (_password_mode)
    {
        return std::string(_text.size(), '*');
    }

    return _text;
}

void TextInput::set_text_internal(const std::string& text, bool notify)
{
    std::string next_text = text.substr(0, _max_length);
    if (_text == next_text)
    {
        return;
    }

    _text = std::move(next_text);
    _caret_index = std::min(_caret_index, _text.size());
    mark_dirty();

    if (notify && _on_text_changed)
    {
        _on_text_changed(_text);
    }
}

void TextInput::move_caret_left()
{
    if (_caret_index > 0)
    {
        --_caret_index;
    }
    _show_caret = true;
    _caret_timer = 0.0;
}

void TextInput::move_caret_right()
{
    if (_caret_index < _text.size())
    {
        ++_caret_index;
    }
    _show_caret = true;
    _caret_timer = 0.0;
}

void TextInput::insert_text(const std::string& text)
{
    if (_read_only || text.empty() || _text.size() >= _max_length)
    {
        return;
    }

    std::string next_text = _text;
    next_text.insert(_caret_index, text);
    if (next_text.size() > _max_length)
    {
        next_text = next_text.substr(0, _max_length);
    }

    _caret_index = std::min(next_text.size(), _caret_index + text.size());
    set_text_internal(next_text, true);
    _show_caret = true;
    _caret_timer = 0.0;
}

void TextInput::erase_left()
{
    if (_read_only || _caret_index == 0 || _text.empty())
    {
        return;
    }

    std::string next_text = _text;
    next_text.erase(_caret_index - 1, 1);
    --_caret_index;
    set_text_internal(next_text, true);
    _show_caret = true;
    _caret_timer = 0.0;
}

void TextInput::erase_right()
{
    if (_read_only || _caret_index >= _text.size())
    {
        return;
    }

    std::string next_text = _text;
    next_text.erase(_caret_index, 1);
    set_text_internal(next_text, true);
    _show_caret = true;
    _caret_timer = 0.0;
}

void TextInput::update_caret_from_mouse(int mouse_x)
{
    TTF_Font* resolved_font = resolve_font();
    if (!resolved_font)
    {
        return;
    }

    const int start_x = rect().x + _padding;
    const std::string visible_text = display_text();
    size_t best_index = 0;
    int best_distance = std::abs(mouse_x - start_x);

    for (size_t index = 1; index <= visible_text.size(); ++index)
    {
        const std::string prefix = visible_text.substr(0, index);
        int prefix_width = 0;
        TTF_SizeUTF8(resolved_font, prefix.c_str(), &prefix_width, nullptr);
        const int distance = std::abs(mouse_x - (start_x + prefix_width));
        if (distance < best_distance)
        {
            best_distance = distance;
            best_index = index;
        }
    }

    _caret_index = std::min(best_index, _text.size());
    _show_caret = true;
    _caret_timer = 0.0;
}

int TextInput::caret_pixel_x() const
{
    TTF_Font* resolved_font = resolve_font();
    if (!resolved_font)
    {
        return rect().x + _padding;
    }

    const std::string prefix = display_text().substr(0, std::min(_caret_index, display_text().size()));
    int prefix_width = 0;
    TTF_SizeUTF8(resolved_font, prefix.c_str(), &prefix_width, nullptr);
    return rect().x + _padding + prefix_width;
}

bool TextInput::contains_point(int x, int y) const
{
    const SDL_Point point{ x, y };
    return SDL_PointInRect(&point, &rect()) == SDL_TRUE;
}
