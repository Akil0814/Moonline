#pragma once
#include "asset_config_types.h"

#include <filesystem>

namespace elysia::io
{
class AssetsStructureLoader
{
public:
	bool load(
		const std::filesystem::path& assets_structure_path,
		AssetManifestPaths& manifest_paths
	) const;
};

}
