#include "json_loader.h"
#include <fstream>

#include <iostream>

bool JsonLoader::bind_file_path(const std::filesystem::path& path)
{
	if (path.empty())
		return false;

	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cout << "JSON open failed: " << std::endl;
		return false;
	}

	try
	{
		file >> _root;
	}
	catch (const std::exception& error)
	{
		return false;
		std::cout << "JSON root failed: " << std::endl;

	}


	_current_file_path = path;
	return true;
}

bool JsonLoader::get_file_path_array(std::string_view key, std::vector<std::filesystem::path>& out) const
{
	if (_root.is_null() || key.empty())
		return false;

	std::string key_string(key);
	if (!_root.contains(key))
		return false;

	if (!_root.at(key).is_array())
		return false;

	out.clear();

	for (const auto& item : _root.at(key))
	{
		if (!item.is_string())
			return false;

		out.emplace_back(item.get<std::string>());
	}

	return true;
}

