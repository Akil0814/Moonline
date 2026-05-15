#include "file_manager.h"
#include "json_loader.h"

#include <string>
#include <string_view>
#include <utility>

bool FileManager::load_assets_strcutre(const std::filesystem::path& assets_structure_path)
{
	_directories.clear();
	_find_all_folder = false;

	JsonLoader loader;
	JsonReadResult result;

	result = loader.open_file(assets_structure_path);
	if (result.success == false)
		return false;

	std::vector<std::pair<std::string, std::string>> directories;
	const std::filesystem::path assets_root = assets_structure_path.parent_path();
	constexpr std::string_view directory_suffix = "_directories";

	try
	{
		if (!loader.root().is_object())
			return false;

		for (const auto& item : loader.root().items())
		{
			if (!item.value().is_array())
				return false;

			std::string group_name = item.key();
			if (group_name.ends_with(directory_suffix))
				group_name.erase(group_name.size() - directory_suffix.size());

			if (group_name.empty())
				return false;

			for (const auto& directory : item.value())
			{
				if (!directory.is_string())
					return false;

				const std::string directory_name = directory.get<std::string>();
				if (directory_name.empty())
					return false;

				std::filesystem::path directory_path = directory_name;
				if (!directory_path.is_absolute())
				{
					if (directory_path.has_parent_path())
						directory_path = assets_root / directory_path;
					else
						directory_path = assets_root / group_name / directory_path;
				}

				directory_path = directory_path.lexically_normal();

				if (!std::filesystem::is_directory(directory_path))
					return false;

				directories.emplace_back(
					group_name,
					directory_path.string()
				);
			}
		}
	}
	catch (const std::filesystem::filesystem_error&)
	{
		return false;
	}

	_directories = std::move(directories);
	_find_all_folder = true;
	return true;
}
