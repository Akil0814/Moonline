#include "resource_bootstrapper.h"

#include <iostream>

bool ResourceBootstrapper::bootstrap(const std::filesystem::path& start_path)
{
	if (!_path_manager.init(start_path))
		return false;

	std::cout << "1: " << std::endl;

	if(!_json_loader.bind_file_path(_path_manager.assets() / "assets_structure.json"))
	{
		std::cout << "JSON bind failed: " << std::endl;
		return false;
	}
	std::cout << "2: " << std::endl;

	if (!_json_loader.get_file_path_array("required_directories", _all_file_path))
	{
		std::cout << "JSON read failed: "  << std::endl;
		return false;
	}
	std::cout << "3: " << std::endl;

	for (const auto& path : _all_file_path)
	{
		std::cout << path << std::endl;
	}
}