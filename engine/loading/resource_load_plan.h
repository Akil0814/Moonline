#pragma once

#include "../resources/resource_types.h"

#include <cstddef>
#include <vector>

class ResourceLoadPlan
{
public:
	std::vector<FontLoadRequest>& font_requests()
	{
		return _font_requests;
	}

	const std::vector<FontLoadRequest>& font_requests() const
	{
		return _font_requests;
	}

	std::vector<SoundLoadRequest>& sound_requests()
	{
		return _sound_requests;
	}

	const std::vector<SoundLoadRequest>& sound_requests() const
	{
		return _sound_requests;
	}

	std::vector<MusicLoadRequest>& music_requests()
	{
		return _music_requests;
	}

	const std::vector<MusicLoadRequest>& music_requests() const
	{
		return _music_requests;
	}

	std::vector<TextureLoadRequest>& texture_requests()
	{
		return _texture_requests;
	}

	const std::vector<TextureLoadRequest>& texture_requests() const
	{
		return _texture_requests;
	}

	std::vector<AtlasBuildRequest>& atlas_build_requests()
	{
		return _atlas_build_requests;
	}

	const std::vector<AtlasBuildRequest>& atlas_build_requests() const
	{
		return _atlas_build_requests;
	}

	std::vector<AnimationBuildRequest>& animation_build_requests()
	{
		return _animation_build_requests;
	}

	const std::vector<AnimationBuildRequest>& animation_build_requests() const
	{
		return _animation_build_requests;
	}

	std::vector<EffectBuildRequest>& effect_build_requests()
	{
		return _effect_build_requests;
	}

	const std::vector<EffectBuildRequest>& effect_build_requests() const
	{
		return _effect_build_requests;
	}

	void clear()
	{
		_font_requests.clear();
		_sound_requests.clear();
		_music_requests.clear();
		_texture_requests.clear();
		_atlas_build_requests.clear();
		_animation_build_requests.clear();
		_effect_build_requests.clear();
	}

	[[nodiscard]] bool empty() const
	{
		return _font_requests.empty()
			&& _sound_requests.empty()
			&& _music_requests.empty()
			&& _texture_requests.empty()
			&& _atlas_build_requests.empty()
			&& _animation_build_requests.empty()
			&& _effect_build_requests.empty();
	}

	[[nodiscard]] std::size_t total_request_count() const
	{
		return _font_requests.size()
			+ _sound_requests.size()
			+ _music_requests.size()
			+ _texture_requests.size()
			+ _atlas_build_requests.size()
			+ _animation_build_requests.size()
			+ _effect_build_requests.size();
	}

private:
	std::vector<FontLoadRequest> _font_requests;
	std::vector<SoundLoadRequest> _sound_requests;
	std::vector<MusicLoadRequest> _music_requests;
	std::vector<TextureLoadRequest> _texture_requests;
	std::vector<AtlasBuildRequest> _atlas_build_requests;
	std::vector<AnimationBuildRequest> _animation_build_requests;
	std::vector<EffectBuildRequest> _effect_build_requests;
};
