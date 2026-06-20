#pragma once

#include "asset_config_types.h"

#include <filesystem>

class I18nManifestLoader
{
public:
	bool load(
		const std::filesystem::path& manifest_path,
		I18nManifest& manifest
	) const;
};
