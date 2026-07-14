#pragma once
#include "../../io/loaders/asset_config_types.h"
#include "../resource_types.h"

#include <vector>

namespace elysia::resources
{
enum class AnimatedEntityAnimationKind
{
	Body,
	Effect
};

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
	bool append_animation_effect_manifest_requests(
		const elysia::io::AnimationEffectManifest& animation_effect_manifest,
		std::vector<AnimationEffectBuildRequest>& animation_effect_build_requests
	) const;
	bool append_animated_entity_texture_requests(
		const elysia::io::AnimatedEntityResourceConfig& entity_config,
		const elysia::io::EntityTextureLayout& texture_layout,
		std::vector<TextureLoadRequest>& texture_load_requests
	) const;
	bool append_animated_entity_audio_requests(
		const elysia::io::AnimatedEntityResourceConfig& entity_config,
		const elysia::io::EntityAudioLayout& audio_layout,
		std::vector<SoundLoadRequest>& sound_load_requests
	) const;
	bool append_animated_entity_animation_requests(
		const elysia::io::AnimatedEntityResourceConfig& entity_config,
		const elysia::io::AnimationConfig& animation_config,
		std::vector<AtlasBuildRequest>& atlas_build_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests,
		AnimatedEntityAnimationKind kind = AnimatedEntityAnimationKind::Body
	) const;
	bool append_animated_entity_effect_definition_requests(
		const elysia::io::AnimatedEntityResourceConfig& entity_config,
		const elysia::io::AnimationConfig& animation_config,
		const elysia::io::EffectDefinitionConfig& effect_config,
		const std::vector<AnimationBuildRequest>& animation_build_requests,
		std::vector<AnimationEffectBuildRequest>& animation_effect_build_requests
	) const;
};


}
