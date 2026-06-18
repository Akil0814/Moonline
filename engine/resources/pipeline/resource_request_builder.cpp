#include "resource_request_builder.h"

#include "../../io/path/path_manager.h"

#include <iostream>
#include <string>
#include <utility>

bool ResourceRequestBuilder::append_font_requests(
	const FontManifest& font_manifest,
	std::vector<FontLoadRequest>& font_load_requests
) const
{
	const std::filesystem::path font_root = PathManager::instance()->fonts();

	for (const FontManifestEntry& entry : font_manifest.fonts)
	{
		if (entry.key.empty())
		{
			std::cout << "Build font requests failed: key is empty." << std::endl;
			return false;
		}

		if (entry.file_path.empty())
		{
			std::cout << "Build font requests failed: file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		if (entry.point_size <= 0)
		{
			std::cout << "Build font requests failed: point size is invalid: "
				<< entry.key << std::endl;
			return false;
		}

		std::filesystem::path file_path = (font_root / entry.file_path).lexically_normal();
		if (file_path.empty())
		{
			std::cout << "Build font requests failed: resolved file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		FontLoadRequest request;
		request.key = entry.key;
		request.file_path = std::move(file_path);
		request.point_size = entry.point_size;
		font_load_requests.push_back(std::move(request));
	}

	return true;
}

bool ResourceRequestBuilder::append_audio_requests(
	const AudioManifest& audio_manifest,
	std::vector<SoundLoadRequest>& sound_load_requests,
	std::vector<MusicLoadRequest>& music_load_requests
) const
{
	const std::filesystem::path audio_root = PathManager::instance()->audio();

	for (const AudioManifestEntry& entry : audio_manifest.sounds)
	{
		if (entry.key.empty())
		{
			std::cout << "Build sound requests failed: key is empty." << std::endl;
			return false;
		}

		if (entry.file_path.empty())
		{
			std::cout << "Build sound requests failed: file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		std::filesystem::path file_path = (audio_root / entry.file_path).lexically_normal();
		if (file_path.empty())
		{
			std::cout << "Build sound requests failed: resolved file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		SoundLoadRequest request;
		request.key = entry.key;
		request.file_path = std::move(file_path);
		sound_load_requests.push_back(std::move(request));
	}

	for (const AudioManifestEntry& entry : audio_manifest.music)
	{
		if (entry.key.empty())
		{
			std::cout << "Build music requests failed: key is empty." << std::endl;
			return false;
		}

		if (entry.file_path.empty())
		{
			std::cout << "Build music requests failed: file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		std::filesystem::path file_path = (audio_root / entry.file_path).lexically_normal();
		if (file_path.empty())
		{
			std::cout << "Build music requests failed: resolved file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		MusicLoadRequest request;
		request.key = entry.key;
		request.file_path = std::move(file_path);
		music_load_requests.push_back(std::move(request));
	}

	return true;
}

bool ResourceRequestBuilder::append_character_animation_requests(
	const CharacterConfig& character_config,
	const AnimationConfig& animation_config,
	std::vector<AtlasLoadRequest>& atlas_load_requests,
	std::vector<AnimationBuildRequest>& animation_build_requests
) const
{
	if (character_config.id.empty())
	{
		std::cout << "Build resource requests failed: character id is empty." << std::endl;
		return false;
	}

	for (const AnimationClipConfig& clip_config : animation_config.clips)
	{
		if (clip_config.animation_name.empty())
		{
			std::cout << "Build resource requests failed: animation name is empty: "
				<< character_config.id << std::endl;
			return false;
		}

		if (clip_config.frame_count == 0 || clip_config.fps <= 0.0)
		{
			std::cout << "Build resource requests failed: animation clip timing is invalid: "
				<< character_config.id << "." << clip_config.animation_name << std::endl;
			return false;
		}

		std::filesystem::path directory_path =
			(character_config.texture_root / clip_config.path).lexically_normal();
		if (directory_path.empty())
		{
			std::cout << "Build resource requests failed: animation directory is empty: "
				<< character_config.id << "." << clip_config.animation_name << std::endl;
			return false;
		}

		std::string animation_key =
			character_config.id + "." + clip_config.animation_name;
		if (clip_config.is_segment)
			animation_key += "." + std::to_string(clip_config.segment_index);

		AtlasLoadRequest atlas_request;
		atlas_request.atlas_key = animation_key;
		atlas_request.directory_path = std::move(directory_path);
		atlas_request.frame_count = clip_config.frame_count;

		AnimationBuildRequest animation_request;
		animation_request.animation_key = animation_key;
		animation_request.atlas_key = atlas_request.atlas_key;
		animation_request.fps = clip_config.fps;
		animation_request.loop = clip_config.loop;
		animation_request.segment_index = clip_config.segment_index;

		atlas_load_requests.push_back(std::move(atlas_request));
		animation_build_requests.push_back(std::move(animation_request));
	}

	return true;
}
