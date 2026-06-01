#pragma once
#include "asset_config_types.h"

#include <filesystem>

class CharacterManifestLoader
{
public:
	bool load(
		const std::filesystem::path& manifest_path,
		CharacterManifest& manifest
	) const;
};
