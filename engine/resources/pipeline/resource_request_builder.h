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
	bool append_character_animation_requests(
		const CharacterConfig& character_config,
		const AnimationConfig& animation_config,
		std::vector<AtlasLoadRequest>& atlas_load_requests,
		std::vector<AnimationBuildRequest>& animation_build_requests
	) const;
};
