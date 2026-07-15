#pragma once

#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <map>
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
	elysia::io::AnimationEffectManifest animation_effect_manifest;

	std::map<std::string, elysia::io::EntityContentModule> additional_modules;
};

class ConfigLoadPipeline
{
public:
	bool load(
		const std::filesystem::path& content_registry_path,
		ConfigLoadResult& result
	);

	const std::string& error_message() const;

private:
	void fail(std::string message);

private:
	std::string _error_message;
};

}
