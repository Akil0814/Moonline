#pragma once
#include "../../io/loaders/asset_config_types.h"
#include "../resource_types.h"

#include <vector>

namespace elysia::resources
{
class ResourceRequestBuilder
{
public:
	bool append_font_requests(
		const elysia::io::FontManifest& font_manifest,
		std::vector<FontLoadRequest>& font_load_requests
	) const;
	bool append_audio_requests(
		const elysia::io::AudioManifest& audio_manifest,
		std::vector<SoundLoadRequest>& sound_load_requests,
		std::vector<MusicLoadRequest>& music_load_requests
	) const;
	bool append_texture_manifest_requests(
		const elysia::io::TextureManifest& texture_manifest,
		const std::filesystem::path& texture_root,
		std::vector<TextureLoadRequest>& texture_load_requests
	) const;
	bool append_animation_manifest_requests(
		const elysia::io::AnimationManifest& animation_manifest,
		const std::filesystem::path& textures_root,
		std::vector<AtlasBuildRequest>& atlas_build_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests
	) const;
	bool append_effect_manifest_requests(
		const elysia::io::EffectManifest& effect_manifest,
		std::vector<EffectBuildRequest>& effect_build_requests
	) const;
	bool append_character_texture_requests(
		const elysia::io::CharacterConfig& character_config,
		const elysia::io::CharacterTextureLayout& texture_layout,
		std::vector<TextureLoadRequest>& texture_load_requests
	) const;
	bool append_character_audio_requests(
		const elysia::io::CharacterConfig& character_config,
		const elysia::io::CharacterAudioLayout& audio_layout,
		std::vector<SoundLoadRequest>& sound_load_requests
	) const;
	bool append_character_animation_requests(
		const elysia::io::CharacterConfig& character_config,
		const elysia::io::AnimationConfig& animation_config,
		std::vector<AtlasBuildRequest>& atlas_build_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests
	) const;
	bool append_character_effect_requests(
		const elysia::io::CharacterConfig& character_config,
		const elysia::io::AnimationConfig& animation_config,
		const elysia::io::CharacterEffectLayout& effect_layout,
		std::vector<AtlasBuildRequest>& atlas_build_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests,
		std::vector<EffectBuildRequest>& effect_build_requests
	) const;
};


}
