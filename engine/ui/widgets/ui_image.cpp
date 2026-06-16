#include "ui_image.h"

#include "../../core/render/render_command.h"

#include <SDL.h>

UiImage::UiImage(SDL_Texture* texture, Vector2 pos, Vector2 size, int order)
    : UiElement(pos, size, order)
{
    set_texture(texture);
}

UiImage::UiImage(SDL_Texture* texture, Rect rect, int order)
    : UiElement(rect, order)
{
    set_texture(texture);
}

UiImage::UiImage(
    SDL_Texture* texture,
    Vector2 center,
    Vector2 image_size,
    UiImageCenterTag,
    int order
)
    : UiElement(Rect::from_center(center, image_size), order)
{
    set_texture(texture);
}

UiImage::UiImage(
    SDL_Texture* texture,
    Vector2 center,
    Vector2 source_size,
    Vector2 render_size,
    UiImageCenterTag,
    int order
)
    : UiElement(Rect::from_center(center, render_size), order)
{
    set_texture(texture);
    set_source_rect(Rect(0.0f, 0.0f, source_size.x, source_size.y));
}

void UiImage::set_texture(SDL_Texture* texture)
{
    _texture = texture;
    if (_texture)
        SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

SDL_Texture* UiImage::texture() const
{
    return _texture;
}

void UiImage::set_source_rect(const Rect& rect)
{
    _has_source_rect = true;
    _source_rect = rect;
}

void UiImage::clear_source_rect()
{
    _has_source_rect = false;
    _source_rect = Rect::zero();
}

void UiImage::set_alpha(std::uint8_t alpha)
{
    _alpha = alpha;
}

std::uint8_t UiImage::alpha() const
{
    return _alpha;
}

void UiImage::submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const
{
    if (!_texture || !is_visible())
        return;

    UiRenderCommand command = make_ui_texture_command(_texture, screen_rect(), _alpha);
    if (_has_source_rect)
    {
        command.use_src_rect = true;
        command.src_rect = _source_rect;
    }

    out_commands.push_back(command);
}
