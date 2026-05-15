#include "character_manifest_loader.h"

#include "json_loader.h"
#include "path_manager.h"

#include <iostream>
#include <string>
#include <utility>

bool CharacterManifestLoader::load(
	const std::filesystem::path& manifest_path,
	CharacterManifest& manifest
) const
{
	manifest = CharacterManifest{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(manifest_path);
	if (!result)
	{
		std::cout << "Load character manifest failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character manifest failed: root is not an object: "
			<< manifest_path << std::endl;
		return false;
	}

	if (!loader.root().contains("characters") || !loader.root().at("characters").is_array())
	{
		std::cout << "Load character manifest failed: characters is missing or not an array: "
			<< manifest_path << std::endl;
		return false;
	}

	CharacterManifest parsed_manifest;
	const json& characters = loader.root().at("characters");
	for (const json& character : characters)
	{
		if (!character.is_object())
		{
			std::cout << "Load character manifest failed: character entry is not an object."
				<< std::endl;
			return false;
		}

		bool enabled = true;
		if (character.contains("enabled"))
		{
			if (!character.at("enabled").is_boolean())
			{
				std::cout << "Load character manifest failed: enabled is not a bool."
					<< std::endl;
				return false;
			}
			enabled = character.at("enabled").get<bool>();
		}

		if (!enabled)
			continue;

		if (!character.contains("id") || !character.at("id").is_string())
		{
			std::cout << "Load character manifest failed: id is missing or not a string."
				<< std::endl;
			return false;
		}

		if (!character.contains("asset_key") || !character.at("asset_key").is_string())
		{
			std::cout << "Load character manifest failed: asset_key is missing or not a string."
				<< std::endl;
			return false;
		}

		if (!character.contains("config") || !character.at("config").is_string())
		{
			std::cout << "Load character manifest failed: config is missing or not a string."
				<< std::endl;
			return false;
		}

		std::filesystem::path config_path =
			PathManager::instance()->resolve_config_path(character.at("config").get<std::string>());
		if (!std::filesystem::is_regular_file(config_path))
		{
			std::cout << "Load character manifest failed: config file does not exist: "
				<< config_path << std::endl;
			return false;
		}

		CharacterManifestEntry entry;
		entry._id = character.at("id").get<std::string>();
		entry._asset_key = character.at("asset_key").get<std::string>();
		entry._config_path = config_path;
		parsed_manifest._characters.push_back(std::move(entry));
	}

	manifest = std::move(parsed_manifest);
	return true;
}
