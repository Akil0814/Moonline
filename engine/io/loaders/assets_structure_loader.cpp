#include "../../tools/logger.h"
#include "assets_structure_loader.h"

#include "../json/json_loader.h"
#include "../path/path_manager.h"
#include <string>
#include <string_view>

namespace elysia::io
{
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
		ELYSIA_LOG_ERROR("io","Load assets structure failed: manifest key is missing: "
			<< key);
		return false;
	}

	const json& path_node = manifests.at(key_string);
	if (!path_node.is_string())
	{
		ELYSIA_LOG_ERROR("io","Load assets structure failed: manifest path is not a string: "
			<< key);
		return false;
	}

	out_path = path_manager.to_asset_path(path_node.get<std::string>());
	if (!std::filesystem::is_regular_file(out_path))
	{
		ELYSIA_LOG_ERROR("io","Load assets structure failed: manifest file does not exist: "
			<< out_path);
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
		|| key == "ui_textures"
		|| key == "config_documents";
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
		ELYSIA_LOG_ERROR("io","Load assets structure failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_ERROR("io","Load assets structure failed: JSON root is not an object: "
			<< assets_structure_path);
		return false;
	}

	if (loader.root().size() != 1 || !loader.root().contains(std::string(manifests_key)))
	{
		for (json::const_iterator item = loader.root().begin(); item != loader.root().end(); ++item)
		{
			if (item.key() != manifests_key)
			{
				ELYSIA_LOG_ERROR("io","Load assets structure failed: unknown root key: "
					<< item.key());
				return false;
			}
		}

		if (!loader.root().contains(std::string(manifests_key)))
		{
			ELYSIA_LOG_ERROR("io","Load assets structure failed: manifests is missing.");
			return false;
		}
	}

	const json& manifests = loader.root().at(std::string(manifests_key));
	if (!manifests.is_object())
	{
		ELYSIA_LOG_ERROR("io","Load assets structure failed: manifests is not an object.");
		return false;
	}

	for (json::const_iterator manifest_item = manifests.begin();
		manifest_item != manifests.end();
		++manifest_item)
	{
		if (!is_known_manifest_key(manifest_item.key()))
		{
			ELYSIA_LOG_ERROR("io","Load assets structure failed: unknown manifest key: "
				<< manifest_item.key());
			return false;
		}
	}

	PathManager* path_manager = PathManager::instance();
	if (!path_manager)
	{
		ELYSIA_LOG_ERROR("io","Load assets structure failed: path manager is null.");
		return false;
	}

	return read_manifest_path(manifests, "characters", *path_manager, manifest_paths.characters)
		&& read_manifest_path(manifests, "character_animations", *path_manager, manifest_paths.character_animations)
		&& read_manifest_path(manifests, "character_audio", *path_manager, manifest_paths.character_audio)
		&& read_manifest_path(manifests, "character_effects", *path_manager, manifest_paths.character_effects)
		&& read_manifest_path(manifests, "character_textures", *path_manager, manifest_paths.character_textures)
		&& read_manifest_path(manifests, "audio", *path_manager, manifest_paths.audio)
		&& read_manifest_path(manifests, "fonts", *path_manager, manifest_paths.fonts)
		&& read_manifest_path(manifests, "i18n", *path_manager, manifest_paths.i18n)
		&& read_manifest_path(manifests, "map_textures", *path_manager, manifest_paths.map_textures)
		&& read_manifest_path(manifests, "ui_textures", *path_manager, manifest_paths.ui_textures)
		&& read_manifest_path(manifests, "config_documents", *path_manager, manifest_paths.config_documents);
}

}
