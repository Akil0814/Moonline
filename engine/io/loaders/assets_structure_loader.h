#pragma once
#include "asset_config_types.h"

#include <filesystem>

class AssetsStructureLoader
{
public:
	bool load(
		const std::filesystem::path& assets_structure_path,
		AssetsStructureManifest& manifest
	) const;
};
