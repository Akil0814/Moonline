#include <SDL.h>

struct RenderCommand
{
    SDL_Texture* texture = nullptr;
    SDL_Rect src_rect{};
    SDL_FRect world_rect{};
    double angle = 0.0;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
};