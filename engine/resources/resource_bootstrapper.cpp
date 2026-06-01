#include "resource_bootstrapper.h"

#include "resource_manager.h"
#include "../animation/animation_manager.h"
#include "../io/path/path_manager.h"

#include <iostream>

bool ResourceBootstrapper::bootstrap(
	const std::filesystem::path& start_path,
	SDL_Renderer* renderer
)
{
	if (!renderer)
	{
		std::cout << "Resource bootstrap failed: renderer is null." << std::endl;
		return false;
	}

	PathManager* path_manager = PathManager::instance();
	if (!path_manager->init(start_path))
	{
		std::cout << "Resource bootstrap failed: path manager init failed: "
			<< start_path << std::endl;
		return false;
	}

	if (!_file_manager.load_assets_strcutre(path_manager->assets_structure()))
	{
		std::cout << "Resource bootstrap failed: assets structure load failed."
			<< std::endl;
		return false;
	}

	if (!_file_manager.load_configs())
	{
		std::cout << "Resource bootstrap failed: config load failed." << std::endl;
		return false;
	}

	ResourceManager* resource_manager = ResourceManager::instance();
	if (!resource_manager->load_atlases(renderer, _file_manager.atlas_load_requests()))
	{
		std::cout << "Resource bootstrap failed: atlas load failed." << std::endl;
		return false;
	}

	AnimationManager* animation_manager = AnimationManager::instance();
	if (!animation_manager->register_animations(
		_file_manager.animation_build_requests(),
		*resource_manager))
	{
		std::cout << "Resource bootstrap failed: animation register failed."
			<< std::endl;
		return false;
	}

	return true;
}
