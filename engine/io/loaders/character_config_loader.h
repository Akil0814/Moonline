#pragma once
#include "asset_config_types.h"

#include <filesystem>
#include <string>

class CharacterConfigLoader
{
public:
	bool load(
		const CharacterManifestEntry& manifest_entry,
		CharacterConfig& config
	) const;

	bool load(
		const std::string& manifest_id,
		const std::string& manifest_asset_key,
		const std::filesystem::path& config_path,
		CharacterConfig& config
	) const;
};
