#pragma once

#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace elysia::loading
{
struct ConfigLoadResult
{
	elysia::io::FontManifest font_manifest;
	elysia::io::AudioManifest audio_manifest;
	elysia::io::TextureManifest map_texture_manifest;
	elysia::io::TextureManifest ui_texture_manifest;
	elysia::io::CharacterEffectLayout character_effect_layout;
	elysia::io::CharacterTextureLayout character_texture_layout;
	elysia::io::CharacterAudioLayout character_audio_layout;
	std::vector<elysia::io::CharacterAnimationContentEntry> character_animation_entries;
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

}
