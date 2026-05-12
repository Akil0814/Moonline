#include "resource_bootstrapper.h"

bool ResourceBootstrapper::bootstrap(const std::filesystem::path& start_path)
{
	if (!_path_manager.init(start_path))
		return false;
}