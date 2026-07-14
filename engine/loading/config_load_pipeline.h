#pragma once

#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace elysia::loading
{
struct ConfigLoadResult
{
	elysia::io::FontManifest font_manifest;
	elysia::io::AudioManifest audio_manifest;
	elysia::io::TextureManifest texture_manifest;
	elysia::io::AnimationManifest animation_manifest;
	elysia::io::EffectManifest effect_manifest;

	struct CharactersContent
	{
		elysia::io::AnimatedEntityContent content;
	};

	std::optional<CharactersContent> characters;
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
