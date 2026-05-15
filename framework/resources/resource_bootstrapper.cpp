#include "resource_bootstrapper.h"

#include <iostream>

bool ResourceBootstrapper::bootstrap(const std::filesystem::path& start_path)
{
	if (!_path_manager.init(start_path))
		return false;
	
	_file_manager.load_config(_path_manager.configs());

	return true;
}
