#include "label.h"

#include "../../resources/resource_manager.h"

#include <algorithm>

Label::Label(Vector2 position, Vector2 size, int order)
    : GameObject(DepthLayer::UI, order)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
}

void Label::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    refresh_texture(renderer);

    const SDL_Rect& object_rect = rect();
    if (_draw_background && object_rect.w > 0 && object_rect.h > 0)
    {
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
        SDL_RenderFillRect(renderer, &object_rect);
        SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
    }

    if (!_texture)
    {
        return;
    }

    const SDL_Rect destination_rect = text_rect();
    if (destination_rect.w <= 0 || destination_rect.h <= 0)
    {
        return;
    }

    SDL_RenderCopy(renderer, _texture.get(), nullptr, &destination_rect);
}

void Label::reset()
{
    GameObject::reset();
    _text.clear();
    _font_key.clear();
    _font = nullptr;
    _texture.reset();
    _text_color = SDL_Color{ 255, 255, 255, 255 };
    _background_color = SDL_Color{ 0, 0, 0, 0 };
    _padding = 0;
    _wrap_width = 0;
    _texture_width = 0;
    _texture_height = 0;
    _draw_background = false;
    _auto_size = true;
    _dirty = true;
    _horizontal_align = TextHorizontalAlign::Left;
    _vertical_align = TextVerticalAlign::Top;
    GameObject::set_size(Vector2::zero());
}

void Label::set_text(const std::string& text)
{
    _text = text;
    mark_dirty();
}

const std::string& Label::text() const
{
    return _text;
}

void Label::set_font(TTF_Font* font)
{
    _font = font;
    mark_dirty();
}

TTF_Font* Label::font() const
{
    return _font;
}

void Label::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    mark_dirty();
}

const std::string& Label::font_key() const
{
    return _font_key;
}

void Label::set_text_color(SDL_Color color)
{
    _text_color = color;
    mark_dirty();
}

SDL_Color Label::text_color() const
{
    return _text_color;
}

void Label::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color Label::background_color() const
{
    return _background_color;
}

void Label::set_draw_background(bool draw_background)
{
    _draw_background = draw_background;
}

bool Label::draws_background() const
{
    return _draw_background;
}

void Label::set_padding(int padding)
{
    _padding = std::max(0, padding);
    mark_dirty();
}

int Label::padding() const
{
    return _padding;
}

void Label::set_auto_size(bool auto_size)
{
    _auto_size = auto_size;
    mark_dirty();
}

bool Label::auto_sizes() const
{
    return _auto_size;
}

void Label::set_horizontal_align(TextHorizontalAlign align)
{
    _horizontal_align = align;
}

TextHorizontalAlign Label::horizontal_align() const
{
    return _horizontal_align;
}

void Label::set_vertical_align(TextVerticalAlign align)
{
    _vertical_align = align;
}

TextVerticalAlign Label::vertical_align() const
{
    return _vertical_align;
}

void Label::set_wrap_width(int wrap_width)
{
    _wrap_width = std::max(0, wrap_width);
    mark_dirty();
}

int Label::wrap_width() const
{
    return _wrap_width;
}

void Label::mark_dirty()
{
    _dirty = true;
}

void Label::refresh_texture(SDL_Renderer* renderer)
{
    if (!_dirty)
    {
        return;
    }

    _dirty = false;
    _texture.reset();
    _texture_width = 0;
    _texture_height = 0;

    if (_text.empty())
    {
        if (_auto_size)
        {
            GameObject::set_size({ static_cast<float>(_padding * 2), static_cast<float>(_padding * 2) });
        }
        return;
    }

    TTF_Font* resolved_font = resolve_font();
    if (!resolved_font)
    {
        return;
    }

    SDL_Surface* text_surface = nullptr;
    if (_wrap_width > 0)
    {
        text_surface = TTF_RenderUTF8_Blended_Wrapped(
            resolved_font,
            _text.c_str(),
            _text_color,
            static_cast<Uint32>(_wrap_width)
        );
    }
    else
    {
        text_surface = TTF_RenderUTF8_Blended(
            resolved_font,
            _text.c_str(),
            _text_color
        );
    }

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

    if (_auto_size)
    {
        GameObject::set_size({
            static_cast<float>(_texture_width + _padding * 2),
            static_cast<float>(_texture_height + _padding * 2)
        });
    }
}

TTF_Font* Label::resolve_font() const
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

SDL_Rect Label::text_rect() const
{
    const SDL_Rect& object_rect = rect();

    SDL_Rect destination_rect{};
    destination_rect.w = _texture_width;
    destination_rect.h = _texture_height;

    const int inner_x = object_rect.x + _padding;
    const int inner_y = object_rect.y + _padding;
    const int inner_width = std::max(0, object_rect.w - _padding * 2);
    const int inner_height = std::max(0, object_rect.h - _padding * 2);

    switch (_horizontal_align)
    {
    case TextHorizontalAlign::Left:
        destination_rect.x = inner_x;
        break;

    case TextHorizontalAlign::Center:
        destination_rect.x = inner_x + (inner_width - destination_rect.w) / 2;
        break;

    case TextHorizontalAlign::Right:
        destination_rect.x = inner_x + inner_width - destination_rect.w;
        break;
    }

    switch (_vertical_align)
    {
    case TextVerticalAlign::Top:
        destination_rect.y = inner_y;
        break;

    case TextVerticalAlign::Center:
        destination_rect.y = inner_y + (inner_height - destination_rect.h) / 2;
        break;

    case TextVerticalAlign::Bottom:
        destination_rect.y = inner_y + inner_height - destination_rect.h;
        break;
    }

    return destination_rect;
}
