#include "character_config_loader.h"

#include "../json/json_loader.h"
#include "../path/path_manager.h"

#include <iostream>

namespace
{
std::filesystem::path infer_texture_root(const std::string& asset_key)
{
	return PathManager::instance()->to_asset_path(
		std::filesystem::path("textures/character") / asset_key
	);
}

std::filesystem::path infer_animation_config_path(const std::filesystem::path& config_path)
{
	return (config_path.parent_path() / "animation_info.json").lexically_normal();
}
}

bool CharacterConfigLoader::load(
	const CharacterManifestEntry& manifest_entry,
	CharacterConfig& config
) const
{
	return load(
		manifest_entry.id,
		manifest_entry.asset_key,
		manifest_entry.config_path,
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

	std::filesystem::path texture_root;
	std::filesystem::path animation_config_path;

	if (loader.root().contains("resources"))
	{
		if (!loader.root().at("resources").is_object())
		{
			std::cout << "Load character config failed: resources is not an object: "
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

		texture_root =
			PathManager::instance()->to_asset_path(resources.at("texture_root").get<std::string>());
		animation_config_path =
			PathManager::instance()->to_config_path(resources.at("animation_config").get<std::string>());
	}
	else
	{
		texture_root = infer_texture_root(asset_key);
		animation_config_path = infer_animation_config_path(config_path);
	}

	if (!std::filesystem::is_directory(texture_root))
	{
		std::cout << "Load character config failed: texture root does not exist: "
			<< texture_root << std::endl;
		return false;
	}

	if (!std::filesystem::is_regular_file(animation_config_path))
	{
		std::cout << "Load character config failed: animation config does not exist: "
			<< animation_config_path << std::endl;
		return false;
	}

	config.id = character_id;
	config.asset_key = asset_key;
	config.texture_root = texture_root;
	config.animation_config_path = animation_config_path;
	return true;
}
