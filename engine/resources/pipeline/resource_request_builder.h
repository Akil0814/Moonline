#pragma once
#include "../../io/loaders/asset_config_types.h"
#include "../resource_types.h"

#include <vector>

class ResourceRequestBuilder
{
public:
	bool append_font_requests(
		const FontManifest& font_manifest,
		std::vector<FontLoadRequest>& font_load_requests
	) const;
	bool append_audio_requests(
		const AudioManifest& audio_manifest,
		std::vector<SoundLoadRequest>& sound_load_requests,
		std::vector<MusicLoadRequest>& music_load_requests
	) const;
	bool append_texture_manifest_requests(
		const TextureManifest& texture_manifest,
		const std::string& key_prefix,
		const std::filesystem::path& texture_root,
		std::vector<TextureLoadRequest>& texture_load_requests
	) const;
	bool append_character_texture_requests(
		const CharacterConfig& character_config,
		const CharacterTextureLayout& texture_layout,
		std::vector<TextureLoadRequest>& texture_load_requests
	) const;
	bool append_character_audio_requests(
		const CharacterConfig& character_config,
		const CharacterAudioLayout& audio_layout,
		std::vector<SoundLoadRequest>& sound_load_requests
	) const;
	bool append_character_animation_requests(
		const CharacterConfig& character_config,
		const AnimationConfig& animation_config,
		std::vector<AtlasBuildRequest>& atlas_build_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests
	) const;
	bool append_character_effect_requests(
		const CharacterConfig& character_config,
		const AnimationConfig& animation_config,
		const CharacterEffectLayout& effect_layout,
		std::vector<AtlasBuildRequest>& atlas_build_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests,
		std::vector<EffectBuildRequest>& effect_build_requests
	) const;
};

