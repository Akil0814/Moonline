#include "../../tools/logger.h"
#include "character_manifest_loader.h"

#include "../json/json_loader.h"
#include "../path/path_manager.h"
#include <string>
#include <utility>

namespace elysia::io
{
namespace
{
std::filesystem::path infer_character_info_path(const std::string& asset_key)
{
	return PathManager::instance()->to_config_path(
		std::filesystem::path("character") / asset_key / "character_info.json"
	);
}
}

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
		ELYSIA_LOG_WARN("io","Load character manifest failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_WARN("io","Load character manifest failed: root is not an object: "
			<< manifest_path);
		return false;
	}

	if (!loader.root().contains("characters") || !loader.root().at("characters").is_array())
	{
		ELYSIA_LOG_WARN("io","Load character manifest failed: characters is missing or not an array: "
			<< manifest_path);
		return false;
	}

	CharacterManifest parsed_manifest;
	const json& characters = loader.root().at("characters");
	for (const json& character : characters)
	{
		if (!character.is_object())
		{
			ELYSIA_LOG_WARN("io","Load character manifest failed: character entry is not an object.");
			return false;
		}

		bool enabled = true;
		if (character.contains("enabled"))
		{
			if (!character.at("enabled").is_boolean())
			{
				ELYSIA_LOG_WARN("io","Load character manifest failed: enabled is not a bool.");
				return false;
			}
			enabled = character.at("enabled").get<bool>();
		}

		if (!enabled)
			continue;

		if (!character.contains("id") || !character.at("id").is_string())
		{
			ELYSIA_LOG_WARN("io","Load character manifest failed: id is missing or not a string.");
			return false;
		}

		if (!character.contains("asset_key") || !character.at("asset_key").is_string())
		{
			ELYSIA_LOG_WARN("io","Load character manifest failed: asset_key is missing or not a string.");
			return false;
		}

		std::filesystem::path config_path;
		if (character.contains("config"))
		{
			if (!character.at("config").is_string())
			{
				ELYSIA_LOG_WARN("io","Load character manifest failed: config is not a string.");
				return false;
			}

			config_path =
				PathManager::instance()->to_config_path(character.at("config").get<std::string>());
		}
		else
		{
			config_path = infer_character_info_path(character.at("asset_key").get<std::string>());
		}

		if (!std::filesystem::is_regular_file(config_path))
		{
			ELYSIA_LOG_WARN("io","Load character manifest failed: config file does not exist: "
				<< config_path);
			return false;
		}

		CharacterManifestEntry entry;
		entry.id = character.at("id").get<std::string>();
		entry.asset_key = character.at("asset_key").get<std::string>();
		entry.config_path = config_path;
		parsed_manifest.characters.push_back(std::move(entry));
	}

	manifest = std::move(parsed_manifest);
	return true;
}

}
