#include "character_config_loader.h"

#include "json_loader.h"
#include "path_manager.h"

#include <iostream>

bool CharacterConfigLoader::load(
	const CharacterManifestEntry& manifest_entry,
	CharacterConfig& config
) const
{
	return load(
		manifest_entry._id,
		manifest_entry._asset_key,
		manifest_entry._config_path,
		config
	);
}

bool CharacterConfigLoader::load(
	const std::string& manifest_id,
	const std::string& manifest_asset_key,
	const std::filesystem::path& config_path,
	CharacterConfig& config
) const
{
	config = CharacterConfig{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(config_path);
	if (!result)
	{
		std::cout << "Load character config failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character config failed: root is not an object: "
			<< config_path << std::endl;
		return false;
	}

	std::string character_id = manifest_id;
	if (loader.root().contains("id") && loader.root().at("id").is_string())
		character_id = loader.root().at("id").get<std::string>();

	std::string asset_key = manifest_asset_key;
	if (loader.root().contains("asset_key") && loader.root().at("asset_key").is_string())
		asset_key = loader.root().at("asset_key").get<std::string>();

	if (character_id.empty() || asset_key.empty())
	{
		std::cout << "Load character config failed: character id or asset key is empty: "
			<< config_path << std::endl;
		return false;
	}

	if (!loader.root().contains("resources") || !loader.root().at("resources").is_object())
	{
		std::cout << "Load character config failed: resources is missing or not an object: "
			<< config_path << std::endl;
		return false;
	}

	const json& resources = loader.root().at("resources");
	if (!resources.contains("texture_root") || !resources.at("texture_root").is_string())
	{
		std::cout << "Load character config failed: texture_root is missing or not a string: "
			<< config_path << std::endl;
		return false;
	}

	if (!resources.contains("animation_config") || !resources.at("animation_config").is_string())
	{
		std::cout << "Load character config failed: animation_config is missing or not a string: "
			<< config_path << std::endl;
		return false;
	}

	std::filesystem::path texture_root =
		PathManager::instance()->resolve_asset_path(resources.at("texture_root").get<std::string>());
	if (!std::filesystem::is_directory(texture_root))
	{
		std::cout << "Load character config failed: texture root does not exist: "
			<< texture_root << std::endl;
		return false;
	}

	std::filesystem::path animation_config_path =
		PathManager::instance()->resolve_config_path(resources.at("animation_config").get<std::string>());
	if (!std::filesystem::is_regular_file(animation_config_path))
	{
		std::cout << "Load character config failed: animation config does not exist: "
			<< animation_config_path << std::endl;
		return false;
	}

	config._id = character_id;
	config._asset_key = asset_key;
	config._texture_root = texture_root;
	config._animation_config_path = animation_config_path;
	return true;
}
