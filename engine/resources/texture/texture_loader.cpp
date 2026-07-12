#include "../../tools/logger.h"
#include "texture_loader.h"
namespace elysia::resources
{
void TextureDeleter::operator()(SDL_Texture* texture) const
{
	if (texture)
		SDL_DestroyTexture(texture);
}

TextureLoadResult TextureLoader::load_texture(
	SDL_Renderer* renderer,
	const SurfaceLoadResult& surface_result
) const
{
	TextureLoadResult result;
	result._asset_key = surface_result._asset_key;
	result._frame_path = surface_result._frame_path;
	result._frame_index = surface_result._frame_index;

	if (!renderer)
	{
		ELYSIA_LOG_ERROR("resource","Load texture failed: renderer is null: "
			<< surface_result._asset_key);
		return result;
	}

	if (!surface_result._success || !surface_result._surface)
	{
		ELYSIA_LOG_ERROR("resource","Load texture failed: surface is invalid: "
			<< surface_result._frame_path);
		return result;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(
		renderer,
		surface_result._surface.get()
	);

	if (!texture)
	{
		ELYSIA_LOG_ERROR("resource","Load texture failed: " << surface_result._frame_path
			<< ", reason: " << SDL_GetError());
		return result;
	}

	result._texture.reset(texture);
	result._success = true;
	return result;
}


}
