#pragma once

#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <string>
#include <vector>

struct ConfigLoadResult
{
	FontManifest font_manifest;
	AudioManifest audio_manifest;
	TextureManifest map_texture_manifest;
	TextureManifest ui_texture_manifest;
	CharacterEffectLayout character_effect_layout;
	CharacterTextureLayout character_texture_layout;
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
