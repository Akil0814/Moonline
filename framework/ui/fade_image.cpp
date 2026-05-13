#include "fade_image.h"

FadeImage::FadeImage(SDL_Texture* texture, Vector2 pos, Vector2 size)
	:GameObject(DepthLayer::UI)
{
	GameObject::set_world_position(pos);
	GameObject::set_size(size);
}