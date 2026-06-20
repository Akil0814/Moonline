#pragma once
#include "asset_config_types.h"

#include <filesystem>

namespace elysia::io
{
class CharacterManifestLoader
{
public:
	bool load(
		const std::filesystem::path& manifest_path,
		CharacterManifest& manifest
	) const;
};

}
