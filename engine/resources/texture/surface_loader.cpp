#include "../../tools/logger.h"
#include "surface_loader.h"

#include <SDL_image.h>
namespace elysia::resources
{
void SurfaceDeleter::operator()(SDL_Surface* surface) const
{
	if (surface)
		SDL_FreeSurface(surface);
}

SurfaceLoadResult SurfaceLoader::load_surface(const SurfaceLoadRequest& request) const
{
	SurfaceLoadResult result;
	result._asset_key = request._asset_key;
	result._frame_path = request._frame_path;
	result._frame_index = request._frame_index;

	if (request._asset_key.empty())
	{
		ELYSIA_LOG_WARN("resource","Load surface failed: asset key is empty.");
		return result;
	}

	if (request._frame_path.empty())
	{
		ELYSIA_LOG_WARN("resource","Load surface failed: frame path is empty: "
			<< request._asset_key);
		return result;
	}

	SDL_Surface* surface = IMG_Load(request._frame_path.string().c_str());
	if (!surface)
	{
		ELYSIA_LOG_WARN("resource","Load surface failed: " << request._frame_path
			<< ", reason: " << IMG_GetError());
		return result;
	}

	result._surface.reset(surface);
	result._success = true;
	return result;
}


}
