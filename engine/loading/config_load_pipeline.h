#pragma once

#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <string>
#include <vector>

struct ConfigLoadResult
{
	FontManifest font_manifest;
	AudioManifest audio_manifest;
	std::vector<CharacterAnimationContentEntry> character_animation_entries;
};

class ConfigLoadPipeline
{
public:
	bool load(
		const std::filesystem::path& assets_structure_path,
		ConfigLoadResult& result
	);

	const std::string& error_message() const;

private:
	void fail(std::string message);

private:
	std::string _error_message;
};
