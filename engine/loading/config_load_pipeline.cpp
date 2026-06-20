#include "config_load_pipeline.h"

#include "../io/loaders/animation_config_loader.h"
#include "../io/loaders/audio_manifest_loader.h"
#include "../io/loaders/assets_structure_loader.h"
#include "../io/loaders/character_animation_layout_loader.h"
#include "../io/loaders/character_audio_layout_loader.h"
#include "../io/loaders/character_config_loader.h"
#include "../io/loaders/character_effect_layout_loader.h"
#include "../io/loaders/character_texture_layout_loader.h"
#include "../io/loaders/character_manifest_loader.h"
#include "../io/loaders/fonts_manifest_loader.h"
#include "../io/loaders/texture_manifest_loader.h"

#include <iostream>
#include <utility>

bool ConfigLoadPipeline::load(
	const std::filesystem::path& assets_structure_path,
	ConfigLoadResult& result
)
{
	result = ConfigLoadResult{};
	_error_message.clear();

	AssetManifestPaths manifest_paths;
	AssetsStructureLoader assets_structure_loader;
	if (!assets_structure_loader.load(assets_structure_path, manifest_paths))
	{
		fail("Config load pipeline failed: assets structure load failed.");
		return false;
	}

	FontsManifestLoader fonts_manifest_loader;
	if (!fonts_manifest_loader.load(manifest_paths.fonts, result.font_manifest))
	{
		fail("Config load pipeline failed: fonts manifest load failed.");
		return false;
	}

	AudioManifestLoader audio_manifest_loader;
	if (!audio_manifest_loader.load(manifest_paths.audio, result.audio_manifest))
	{
		fail("Config load pipeline failed: audio manifest load failed.");
		return false;
	}

	TextureManifestLoader texture_manifest_loader;
	if (!texture_manifest_loader.load(manifest_paths.map_textures, result.map_texture_manifest))
	{
		fail("Config load pipeline failed: map textures manifest load failed.");
		return false;
	}

	if (!texture_manifest_loader.load(manifest_paths.ui_textures, result.ui_texture_manifest))
	{
		fail("Config load pipeline failed: ui textures manifest load failed.");
		return false;
	}

	CharacterAnimationLayout character_animation_layout;
	CharacterAnimationLayoutLoader character_animation_layout_loader;
	if (!character_animation_layout_loader.load(
		manifest_paths.character_animations,
		character_animation_layout))
	{
		fail("Config load pipeline failed: character animation layout load failed.");
		return false;
	}

	CharacterEffectLayoutLoader character_effect_layout_loader;
	if (!character_effect_layout_loader.load(
		manifest_paths.character_effects,
		result.character_effect_layout))
	{
		fail("Config load pipeline failed: character effect layout load failed.");
		return false;
	}

	CharacterTextureLayoutLoader character_texture_layout_loader;
	if (!character_texture_layout_loader.load(
		manifest_paths.character_textures,
		result.character_texture_layout))
	{
		fail("Config load pipeline failed: character texture layout load failed.");
		return false;
	}

	CharacterAudioLayoutLoader character_audio_layout_loader;
	if (!character_audio_layout_loader.load(
		manifest_paths.character_audio,
		result.character_audio_layout))
	{
		fail("Config load pipeline failed: character audio layout load failed.");
		return false;
	}

	CharacterManifest character_manifest;
	CharacterManifestLoader character_manifest_loader;
	if (!character_manifest_loader.load(manifest_paths.characters, character_manifest))
	{
		fail("Config load pipeline failed: character manifest load failed.");
		return false;
	}

	CharacterConfigLoader character_config_loader;
	AnimationConfigLoader animation_config_loader;

	for (const CharacterManifestEntry& character_entry : character_manifest.characters)
	{
		CharacterConfig character_config;
		if (!character_config_loader.load(character_entry, character_config))
		{
			fail("Config load pipeline failed: character config load failed.");
			return false;
		}

		AnimationConfig animation_config;
		if (!animation_config_loader.load(
			character_config.animation_config_path,
			character_animation_layout,
			animation_config))
		{
			fail("Config load pipeline failed: animation config load failed.");
			return false;
		}

		result.character_animation_entries.push_back(CharacterAnimationContentEntry{
			std::move(character_config),
			std::move(animation_config)
		});
	}

	return true;
}

const std::string& ConfigLoadPipeline::error_message() const
{
	return _error_message;
}

void ConfigLoadPipeline::fail(std::string message)
{
	_error_message = std::move(message);
	std::cout << _error_message << std::endl;
}
