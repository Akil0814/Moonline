#include "resource_bootstrapper.h"

#include "resource_manager.h"
#include "../animation/animation_manager.h"
#include "../io/path/path_manager.h"

#include <iostream>

bool ResourceBootstrapper::bootstrap(SDL_Renderer* renderer)
{
	if (!renderer)
	{
		std::cout << "Resource bootstrap failed: renderer is null." << std::endl;
		return false;
	}

	PathManager* path_manager = PathManager::instance();
	if (!path_manager->init())
	{
		std::cout << "Resource bootstrap failed: path manager init failed: " << std::endl;
		return false;
	}

	return true;
}
