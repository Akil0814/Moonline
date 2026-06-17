#include "assets_structure_loader.h"

#include "../json/json_loader.h"
#include "../path/path_manager.h"

#include <iostream>
#include <string>
#include <string_view>

namespace
{
constexpr std::string_view manifests_key = "manifests";

bool read_manifest_path(
	const json& manifests,
	std::string_view key,
	PathManager& path_manager,
	std::filesystem::path& out_path
)
{
	const std::string key_string(key);
	if (!manifests.contains(key_string))
	{
		std::cout << "Load assets structure failed: manifest key is missing: "
			<< key << std::endl;
		return false;
	}

	const json& path_node = manifests.at(key_string);
	if (!path_node.is_string())
	{
		std::cout << "Load assets structure failed: manifest path is not a string: "
			<< key << std::endl;
		return false;
	}

	out_path = path_manager.to_asset_path(path_node.get<std::string>());
	if (!std::filesystem::is_regular_file(out_path))
	{
		std::cout << "Load assets structure failed: manifest file does not exist: "
			<< out_path << std::endl;
		return false;
	}

	return true;
}

bool is_known_manifest_key(std::string_view key)
{
	return key == "characters"
		|| key == "character_animations"
		|| key == "character_audio"
		|| key == "character_effects"
		|| key == "character_textures"
		|| key == "audio"
		|| key == "fonts"
		|| key == "i18n"
		|| key == "map_textures"
		|| key == "preload"
		|| key == "ui_textures";
}
}

bool AssetsStructureLoader::load(
	const std::filesystem::path& assets_structure_path,
	AssetManifestPaths& manifest_paths
) const
{
	manifest_paths = AssetManifestPaths{};

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

	if (loader.root().size() != 1 || !loader.root().contains(std::string(manifests_key)))
	{
		for (json::const_iterator item = loader.root().begin(); item != loader.root().end(); ++item)
		{
			if (item.key() != manifests_key)
			{
				std::cout << "Load assets structure failed: unknown root key: "
					<< item.key() << std::endl;
				return false;
			}
		}

		if (!loader.root().contains(std::string(manifests_key)))
		{
			std::cout << "Load assets structure failed: manifests is missing." << std::endl;
			return false;
		}
	}

	const json& manifests = loader.root().at(std::string(manifests_key));
	if (!manifests.is_object())
	{
		std::cout << "Load assets structure failed: manifests is not an object."
			<< std::endl;
		return false;
	}

	for (json::const_iterator manifest_item = manifests.begin();
		manifest_item != manifests.end();
		++manifest_item)
	{
		if (!is_known_manifest_key(manifest_item.key()))
		{
			std::cout << "Load assets structure failed: unknown manifest key: "
				<< manifest_item.key() << std::endl;
			return false;
		}
	}

	PathManager* path_manager = PathManager::instance();
	if (!path_manager)
	{
		std::cout << "Load assets structure failed: path manager is null." << std::endl;
		return false;
	}

	return read_manifest_path(manifests, "characters", *path_manager, manifest_paths._characters)
		&& read_manifest_path(manifests, "character_animations", *path_manager, manifest_paths._character_animations)
		&& read_manifest_path(manifests, "character_audio", *path_manager, manifest_paths._character_audio)
		&& read_manifest_path(manifests, "character_effects", *path_manager, manifest_paths._character_effects)
		&& read_manifest_path(manifests, "character_textures", *path_manager, manifest_paths._character_textures)
		&& read_manifest_path(manifests, "audio", *path_manager, manifest_paths._audio)
		&& read_manifest_path(manifests, "fonts", *path_manager, manifest_paths._fonts)
		&& read_manifest_path(manifests, "i18n", *path_manager, manifest_paths._i18n)
		&& read_manifest_path(manifests, "map_textures", *path_manager, manifest_paths._map_textures)
		&& read_manifest_path(manifests, "preload", *path_manager, manifest_paths._preload)
		&& read_manifest_path(manifests, "ui_textures", *path_manager, manifest_paths._ui_textures);
}
