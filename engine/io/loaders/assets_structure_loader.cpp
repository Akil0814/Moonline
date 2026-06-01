#include "assets_structure_loader.h"

#include "../json/json_loader.h"
#include "../path/path_manager.h"

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace
{
	constexpr std::string_view directory_suffix = "_directories";
}

bool AssetsStructureLoader::load(
	const std::filesystem::path& assets_structure_path,
	AssetsStructureManifest& manifest
) const
{
	manifest = AssetsStructureManifest{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(assets_structure_path);
	if (!result)
	{
		std::cout << "Load assets structure failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load assets structure failed: JSON root is not an object: "
			<< assets_structure_path << std::endl;
		return false;
	}

	PathManager* path_manager = PathManager::instance();
	AssetsStructureManifest parsed_manifest;

	try
	{
		for (json::const_iterator item = loader.root().begin();
			item != loader.root().end();
			++item)
		{
			std::string item_key = item.key();
			const json& item_value = item.value();

			if (item_key.ends_with(directory_suffix))
			{
				if (!item_value.is_array())
				{
					std::cout << "Load assets structure failed: key is not an array: "
						<< item_key << std::endl;
					return false;
				}

				std::string group_name = item_key;
				group_name.erase(group_name.size() - directory_suffix.size());

				if (group_name.empty())
				{
					std::cout << "Load assets structure failed: empty directory group from key: "
						<< item_key << std::endl;
					return false;
				}

				for (const json& directory : item_value)
				{
					if (!directory.is_string())
					{
						std::cout << "Load assets structure failed: directory entry is not a string in key: "
							<< item_key << std::endl;
						return false;
					}

					const std::string directory_name = directory.get<std::string>();
					if (directory_name.empty())
					{
						std::cout << "Load assets structure failed: empty directory entry in key: "
							<< item_key << std::endl;
						return false;
					}

					std::filesystem::path directory_path = directory_name;
					if (!directory_path.is_absolute())
					{
						if (directory_path.has_parent_path())
							directory_path = path_manager->resolve_asset_path(directory_path);
						else
							directory_path = path_manager->resolve_asset_path(
								std::filesystem::path(group_name) / directory_path
							);
					}

					directory_path = directory_path.lexically_normal();
					if (!std::filesystem::is_directory(directory_path))
					{
						std::cout << "Load assets structure failed: directory does not exist: "
							<< directory_path << std::endl;
						return false;
					}

					AssetDirectoryEntry entry;
					entry._group_name = group_name;
					entry._directory_path = directory_path;
					parsed_manifest._directories.push_back(std::move(entry));
				}

				continue;
			}

			if (item_key == "manifests")
			{
				if (!item_value.is_object())
				{
					std::cout << "Load assets structure failed: manifests is not an object."
						<< std::endl;
					return false;
				}

				for (json::const_iterator manifest_item = item_value.begin();
					manifest_item != item_value.end();
					++manifest_item)
				{
					if (!manifest_item.value().is_string())
					{
						std::cout << "Load assets structure failed: manifest path is not a string: "
							<< manifest_item.key() << std::endl;
						return false;
					}

					std::filesystem::path manifest_path =
						path_manager->resolve_asset_path(manifest_item.value().get<std::string>());
					if (!std::filesystem::is_regular_file(manifest_path))
					{
						std::cout << "Load assets structure failed: manifest file does not exist: "
							<< manifest_path << std::endl;
						return false;
					}

					parsed_manifest._manifest_paths.emplace(manifest_item.key(), manifest_path);
				}

				continue;
			}

			std::cout << "Load assets structure failed: unknown root key: "
				<< item_key << std::endl;
			return false;
		}
	}
	catch (const std::filesystem::filesystem_error&)
	{
		std::cout << "Load assets structure failed: filesystem error while reading: "
			<< assets_structure_path << std::endl;
		return false;
	}

	manifest = std::move(parsed_manifest);
	return true;
}
