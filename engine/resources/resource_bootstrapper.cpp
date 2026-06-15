#include "resource_bootstrapper.h"

#include "resource_manager.h"
#include "texture/surface_loader.h"
#include "texture/texture_loader.h"
#include "../io/path/path_manager.h"

#include <algorithm>
#include <iostream>

bool ResourceBootstrapper::bootstrap(SDL_Renderer* renderer)
{
	if (!renderer)
	{
		std::cout << "Resource bootstrap failed: renderer is null." << std::endl;
		return false;
	}

	if (!PathManager::instance()->init())
	{
		std::cout << "Resource bootstrap failed: path manager init failed." << std::endl;
		return false;
	}

	if (!load_preload_manifest())
	{
		std::cout << "Resource bootstrap failed: load preload resource failed." << std::endl;
		return false;
	}

	if (!load_preload_textures(renderer))
	{
		std::cout << "Resource bootstrap failed: load preload texture failed." << std::endl;
		return false;
	}

	return true;
}

const std::vector<Error>& ResourceBootstrapper::get_error_list() const
{
	return _error_list;
}

bool ResourceBootstrapper::load_preload_manifest()
{
	PathManager* path_manager = PathManager::instance();
	const std::filesystem::path preload_manifest =
		path_manager->preload() / "preload_manifest.json";

	const JsonReadResult result = _json_loader.open_file(preload_manifest);
	if (!result.success)
	{
		std::cout << "Load preload resource failed: " << result.error;
		return false;
	}

	return true;
}

SDL_Texture* ResourceBootstrapper::get_preload_texture(std::string_view key)
{
	const std::string requested_key(key);
	if (std::find(
		_preloaded_texture_keys.begin(),
		_preloaded_texture_keys.end(),
		requested_key) == _preloaded_texture_keys.end())
	{
		std::cout << "Texture: " << requested_key << " is not preloaded" << std::endl;
		return nullptr;
	}

	return ResourceManager::instance()->find_texture(key);
}

bool ResourceBootstrapper::load_preload_textures(SDL_Renderer* renderer)
{
	_preloaded_texture_keys.clear();

	std::vector<std::string> texture_paths;
	const JsonReadResult array_result = _json_loader.get_array("textures", texture_paths);
	if (!array_result.success)
	{
		std::cout << "Load preload texture failed: " << array_result.error;
		return false;
	}

	PathManager* path_manager = PathManager::instance();
	ResourceManager* resource_manager = ResourceManager::instance();
	SurfaceLoader surface_loader;
	TextureLoader texture_loader;

	for (const std::string& relative_path : texture_paths)
	{
		if (relative_path.empty())
		{
			std::cout << "Load preload texture failed: texture path is empty." << std::endl;
			return false;
		}

		SurfaceLoadRequest surface_request;
		surface_request._asset_key = relative_path;
		surface_request._frame_path = path_manager->preload() / std::filesystem::path(relative_path);
		surface_request._frame_index = 0;

		SurfaceLoadResult surface_result = surface_loader.load_surface(surface_request);
		if (!surface_result._success)
			return false;

		TextureLoadResult texture_result = texture_loader.load_texture(renderer, surface_result);
		if (!texture_result._success)
			return false;

		if (!resource_manager->texture_manager().store_texture(
			relative_path,
			std::move(texture_result._texture)))
		{
			std::cout << "Load preload texture failed: store texture failed: "
				<< relative_path << std::endl;
			return false;
		}

		_preloaded_texture_keys.push_back(relative_path);
		std::cout << relative_path << std::endl;
	}

	return true;
}
