#pragma once
#include<vector>
#include <filesystem>
#include <string>
#include <utility>

class FileManager
{
public:
	bool load_assets_strcutre(const std::filesystem::path& assets_structure_path);
	bool load_config(const std::filesystem::path& config_path);
private:
	
	std::vector<std::pair<std::string, std::string>> _directories;
	bool _find_all_folder = false;
};
