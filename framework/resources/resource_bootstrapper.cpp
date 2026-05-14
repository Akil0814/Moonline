#include "resource_bootstrapper.h"

#include <iostream>

bool ResourceBootstrapper::bootstrap(const std::filesystem::path& start_path)
{
	if (!_path_manager.init(start_path))
		return false;

	std::cout << "1: " << std::endl;

	JsonReadResult json_result = _json_loader.open_file(_path_manager.assets() / "assets_structure.json");
	if (!json_result)
	{
		std::cout << "JSON open failed: " << json_result.error << std::endl;
		return false;
	}
	std::cout << "2: " << std::endl;

	json_result = _json_loader.get_array("required_directories", _all_file_path);
	if (!json_result)
	{
		std::cout << "JSON read failed: " << json_result.error << std::endl;
		return false;
	}
	std::cout << "3: " << std::endl;

	for (const auto& path : _all_file_path)
	{
		std::cout << path << std::endl;
	}

	return true;
}
