#include "ui_label.h"

#include "../style/ui_theme.h"
#include "../style/ui_style.h"

#include <algorithm>

UiLabel::UiLabel(Vector2 position, Vector2 size, int order)
    : UiElement(position, size, order)
{
}

void UiLabel::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    refresh_theme_if_needed();
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

void UiLabel::reset()
{
    UiElement::reset();
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
    _label_theme_role = UiLabelThemeRole::Default;
    GameObject::set_size(Vector2::zero());
}

void UiLabel::set_text(const std::string& text)
{
    _text = text;
    mark_dirty();
}

const std::string& UiLabel::text() const
{
    return _text;
}

void UiLabel::set_font(TTF_Font* font)
{
    _font = font;
    mark_dirty();
}

TTF_Font* UiLabel::font() const
{
    return _font;
}

void UiLabel::set_font_key(const std::string& font_key)
{
    _font_key = font_key;
    mark_dirty();
}

const std::string& UiLabel::font_key() const
{
    return _font_key;
}

void UiLabel::set_text_color(SDL_Color color)
{
    _text_color = color;
    mark_dirty();
}

SDL_Color UiLabel::text_color() const
{
    return _text_color;
}

void UiLabel::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color UiLabel::background_color() const
{
    return _background_color;
}

void UiLabel::set_draw_background(bool draw_background)
{
    _draw_background = draw_background;
}

bool UiLabel::draws_background() const
{
    return _draw_background;
}

void UiLabel::set_padding(int padding)
{
    _padding = std::max(0, padding);
    mark_dirty();
}

int UiLabel::padding() const
{
    return _padding;
}

void UiLabel::set_auto_size(bool auto_size)
{
    _auto_size = auto_size;
    mark_dirty();
}

bool UiLabel::auto_sizes() const
{
    return _auto_size;
}

void UiLabel::set_horizontal_align(TextHorizontalAlign align)
{
    _horizontal_align = align;
}

TextHorizontalAlign UiLabel::horizontal_align() const
{
    return _horizontal_align;
}

void UiLabel::set_vertical_align(TextVerticalAlign align)
{
    _vertical_align = align;
}

TextVerticalAlign UiLabel::vertical_align() const
{
    return _vertical_align;
}

void UiLabel::set_wrap_width(int wrap_width)
{
    _wrap_width = std::max(0, wrap_width);
    mark_dirty();
}

int UiLabel::wrap_width() const
{
    return _wrap_width;
}

void UiLabel::set_label_theme_role(UiLabelThemeRole label_theme_role)
{
    _label_theme_role = label_theme_role;
    mark_theme_dirty();
}

UiLabelThemeRole UiLabel::label_theme_role() const
{
    return _label_theme_role;
}

void UiLabel::mark_dirty()
{
    _dirty = true;
}

void UiLabel::refresh_texture(SDL_Renderer* renderer)
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

    TTF_Font* resolved_font = ui_resolve_font(_font, _font_key);
    if (!resolved_font)
    {
        return;
    }

    UiTextTextureResult texture_result = ui_render_text_texture(
        renderer,
        resolved_font,
        _text,
        _text_color,
        _wrap_width
    );
    if (!texture_result._texture)
    {
        return;
    }

    _texture_width = texture_result._width;
    _texture_height = texture_result._height;
    _texture = std::move(texture_result._texture);

    if (_auto_size)
    {
        GameObject::set_size({
            static_cast<float>(_texture_width + _padding * 2),
            static_cast<float>(_texture_height + _padding * 2)
        });
    }
}

SDL_Rect UiLabel::text_rect() const
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

void UiLabel::apply_theme(const UiTheme& theme)
{
    const LabelStyle* style = &theme._default_label;
    switch (_label_theme_role)
    {
    case UiLabelThemeRole::Default:
        style = &theme._default_label;
        break;

    case UiLabelThemeRole::Title:
        style = &theme._title_label;
        break;

    case UiLabelThemeRole::Subtitle:
        style = &theme._subtitle_label;
        break;

    case UiLabelThemeRole::Muted:
        style = &theme._muted_label;
        break;
    }

    set_text_color(style->_text_color);
    set_background_color(style->_background_color);
    set_draw_background(style->_draw_background);
}
