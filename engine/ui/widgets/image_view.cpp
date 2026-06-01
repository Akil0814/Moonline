#include "image_view.h"

#include "../../resources/resource_manager.h"

#include <algorithm>

ImageView::ImageView(Vector2 position, Vector2 size, int order)
    : GameObject(DepthLayer::UI, order)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
}

void ImageView::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    SDL_Texture* resolved_texture = resolve_texture();
    if (!resolved_texture)
    {
        return;
    }

    int texture_width = 0;
    int texture_height = 0;
    if (SDL_QueryTexture(resolved_texture, nullptr, nullptr, &texture_width, &texture_height) != 0)
    {
        return;
    }

    SDL_Rect source_rect{};
    SDL_Rect* source_rect_ptr = nullptr;
    if (_source_rect.has_value())
    {
        source_rect = _source_rect.value();
        source_rect_ptr = &source_rect;
        texture_width = source_rect.w;
        texture_height = source_rect.h;
    }

    if (_auto_size)
    {
        GameObject::set_size({
            static_cast<float>(texture_width),
            static_cast<float>(texture_height)
        });
    }

    SDL_Rect destination = destination_rect(texture_width, texture_height);
    if (destination.w <= 0 || destination.h <= 0)
    {
        return;
    }

    Uint8 old_r = 255;
    Uint8 old_g = 255;
    Uint8 old_b = 255;
    Uint8 old_a = 255;
    SDL_GetTextureColorMod(resolved_texture, &old_r, &old_g, &old_b);
    SDL_GetTextureAlphaMod(resolved_texture, &old_a);

    SDL_SetTextureColorMod(
        resolved_texture,
        _tint_color.r,
        _tint_color.g,
        _tint_color.b
    );
    SDL_SetTextureAlphaMod(resolved_texture, _alpha);
    SDL_RenderCopy(renderer, resolved_texture, source_rect_ptr, &destination);
    SDL_SetTextureColorMod(resolved_texture, old_r, old_g, old_b);
    SDL_SetTextureAlphaMod(resolved_texture, old_a);
}

void ImageView::reset()
{
    GameObject::reset();
    _texture = nullptr;
    _texture_key.clear();
    _source_rect.reset();
    _tint_color = SDL_Color{ 255, 255, 255, 255 };
    _alpha = 255;
    _scale_mode = ImageScaleMode::Stretch;
    _auto_size = false;
}

void ImageView::set_texture(SDL_Texture* texture)
{
    _texture = texture;
}

SDL_Texture* ImageView::texture() const
{
    return _texture;
}

void ImageView::set_texture_key(const std::string& texture_key)
{
    _texture_key = texture_key;
}

const std::string& ImageView::texture_key() const
{
    return _texture_key;
}

void ImageView::set_source_rect(const SDL_Rect& source_rect)
{
    _source_rect = source_rect;
}

void ImageView::clear_source_rect()
{
    _source_rect.reset();
}

bool ImageView::has_source_rect() const
{
    return _source_rect.has_value();
}

void ImageView::set_tint_color(SDL_Color tint_color)
{
    _tint_color = tint_color;
}

SDL_Color ImageView::tint_color() const
{
    return _tint_color;
}

void ImageView::set_alpha(Uint8 alpha)
{
    _alpha = alpha;
}

Uint8 ImageView::alpha() const
{
    return _alpha;
}

void ImageView::set_scale_mode(ImageScaleMode scale_mode)
{
    _scale_mode = scale_mode;
}

ImageScaleMode ImageView::scale_mode() const
{
    return _scale_mode;
}

void ImageView::set_auto_size(bool auto_size)
{
    _auto_size = auto_size;
}

bool ImageView::auto_sizes() const
{
    return _auto_size;
}

SDL_Texture* ImageView::resolve_texture() const
{
    if (_texture)
    {
        return _texture;
    }

    if (_texture_key.empty())
    {
        return nullptr;
    }

    return ResourceManager::instance()->find_texture(_texture_key);
}

SDL_Rect ImageView::destination_rect(int texture_width, int texture_height) const
{
    SDL_Rect destination = rect();
    if (_scale_mode == ImageScaleMode::Stretch)
    {
        return destination;
    }

    if (_scale_mode == ImageScaleMode::None)
    {
        destination.w = texture_width;
        destination.h = texture_height;
        return destination;
    }

    if (destination.w <= 0 || destination.h <= 0 || texture_width <= 0 || texture_height <= 0)
    {
        return destination;
    }

    const float width_ratio = static_cast<float>(destination.w) / static_cast<float>(texture_width);
    const float height_ratio = static_cast<float>(destination.h) / static_cast<float>(texture_height);
    const float scale = std::min(width_ratio, height_ratio);

    const int draw_width = std::max(0, static_cast<int>(texture_width * scale));
    const int draw_height = std::max(0, static_cast<int>(texture_height * scale));

    destination.x += (destination.w - draw_width) / 2;
    destination.y += (destination.h - draw_height) / 2;
    destination.w = draw_width;
    destination.h = draw_height;
    return destination;
}
