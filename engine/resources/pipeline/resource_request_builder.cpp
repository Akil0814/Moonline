#include "resource_request_builder.h"

#include "../../io/path/path_manager.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace elysia::resources
{
namespace
{
std::filesystem::path resolve_segment_path(
	const std::filesystem::path& segment_path,
	size_t segment_index
)
{
	std::string path_string = segment_path.string();
	std::string segment_number = std::to_string(segment_index + 1);

	size_t marker_position = path_string.find("{segment}");
	if (marker_position != std::string::npos)
	{
		path_string.replace(marker_position, std::string("{segment}").size(), segment_number);
		return path_string;
	}

	return (segment_path / segment_number).lexically_normal();
}

bool append_texture_request(
	const std::string& key,
	const std::filesystem::path& file_path,
	std::vector<TextureLoadRequest>& texture_load_requests,
	const char* error_prefix
)
{
	if (key.empty())
	{
		std::cout << error_prefix << ": key is empty." << std::endl;
		return false;
	}

	if (file_path.empty())
	{
		std::cout << error_prefix << ": file path is empty: " << key << std::endl;
		return false;
	}

	if (!std::filesystem::is_regular_file(file_path))
	{
		std::cout << error_prefix << ": file does not exist: " << file_path << std::endl;
		return false;
	}

	TextureLoadRequest request;
	request.key = key;
	request.file_path = file_path;
	texture_load_requests.push_back(std::move(request));
	return true;
}

bool append_directory_texture_requests(
	const std::string& base_key,
	const std::filesystem::path& directory_path,
	std::vector<TextureLoadRequest>& texture_load_requests,
	const char* error_prefix
)
{
	if (base_key.empty())
	{
		std::cout << error_prefix << ": base key is empty." << std::endl;
		return false;
	}

	if (!std::filesystem::is_directory(directory_path))
	{
		std::cout << error_prefix << ": directory does not exist: " << directory_path << std::endl;
		return false;
	}

	std::vector<std::filesystem::path> file_paths;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory_path))
	{
		if (entry.is_regular_file())
			file_paths.push_back(entry.path());
	}

	std::sort(file_paths.begin(), file_paths.end());
	if (file_paths.empty())
	{
		std::cout << error_prefix << ": directory has no regular files: "
			<< directory_path << std::endl;
		return false;
	}

	for (const std::filesystem::path& file_path : file_paths)
	{
		const std::string key = base_key + "." + file_path.stem().string();
		if (!append_texture_request(key, file_path, texture_load_requests, error_prefix))
			return false;
	}

	return true;
}
}

bool ResourceRequestBuilder::append_font_requests(
	const elysia::io::FontManifest& font_manifest,
	std::vector<FontLoadRequest>& font_load_requests
) const
{
	const std::filesystem::path font_root = elysia::io::PathManager::instance()->fonts();

	for (const elysia::io::FontManifestEntry& entry : font_manifest.fonts)
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

		std::filesystem::path file_path = (font_root / entry.file_path).lexically_normal();
		if (file_path.empty())
		{
			std::cout << "Build font requests failed: resolved file path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		for (const int point_size : font_manifest.point_sizes)
		{
			if (point_size <= 0)
			{
				std::cout << "Build font requests failed: point size is invalid: "
					<< point_size << std::endl;
				return false;
			}

			FontLoadRequest request;
			request.key = entry.key + "." + std::to_string(point_size);
			request.file_path = file_path;
			request.point_size = point_size;
			font_load_requests.push_back(std::move(request));
		}
	}

	return true;
}

bool ResourceRequestBuilder::append_audio_requests(
	const elysia::io::AudioManifest& audio_manifest,
	std::vector<SoundLoadRequest>& sound_load_requests,
	std::vector<MusicLoadRequest>& music_load_requests
) const
{
	const std::filesystem::path audio_root = elysia::io::PathManager::instance()->audio();

	for (const elysia::io::AudioManifestEntry& entry : audio_manifest.sounds)
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

	for (const elysia::io::AudioManifestEntry& entry : audio_manifest.music)
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

bool ResourceRequestBuilder::append_texture_manifest_requests(
	const elysia::io::TextureManifest& texture_manifest,
	const std::string& key_prefix,
	const std::filesystem::path& texture_root,
	std::vector<TextureLoadRequest>& texture_load_requests
) const
{
	if (key_prefix.empty())
	{
		std::cout << "Build texture requests failed: key prefix is empty." << std::endl;
		return false;
	}

	if (texture_root.empty())
	{
		std::cout << "Build texture requests failed: texture root is empty: "
			<< key_prefix << std::endl;
		return false;
	}

	for (const elysia::io::TextureManifestEntry& entry : texture_manifest.textures)
	{
		const std::string key = key_prefix + "." + entry.key;
		const std::filesystem::path file_path = (texture_root / entry.file_path).lexically_normal();
		if (!append_texture_request(
			key,
			file_path,
			texture_load_requests,
			"Build texture requests failed"))
		{
			return false;
		}
	}

	return true;
}

bool ResourceRequestBuilder::append_character_texture_requests(
	const elysia::io::CharacterConfig& character_config,
	const elysia::io::CharacterTextureLayout& texture_layout,
	std::vector<TextureLoadRequest>& texture_load_requests
) const
{
	if (character_config.id.empty())
	{
		std::cout << "Build character texture requests failed: character id is empty." << std::endl;
		return false;
	}

	if (character_config.texture_root.empty())
	{
		std::cout << "Build character texture requests failed: texture root is empty: "
			<< character_config.id << std::endl;
		return false;
	}

	for (const elysia::io::CharacterTextureLayoutEntry& entry : texture_layout.textures)
	{
		const std::string base_key = character_config.id + "." + entry.key;
		const std::filesystem::path resolved_path =
			(character_config.texture_root / entry.path).lexically_normal();

		if (std::filesystem::is_regular_file(resolved_path))
		{
			if (!append_texture_request(
				base_key,
				resolved_path,
				texture_load_requests,
				"Build character texture requests failed"))
			{
				return false;
			}

			continue;
		}

		if (std::filesystem::is_directory(resolved_path))
		{
			if (!append_directory_texture_requests(
				base_key,
				resolved_path,
				texture_load_requests,
				"Build character texture requests failed"))
			{
				return false;
			}

			continue;
		}

		std::cout << "Build character texture requests failed: target does not exist: "
			<< resolved_path << std::endl;
		return false;
	}

	return true;
}

bool ResourceRequestBuilder::append_character_audio_requests(
	const elysia::io::CharacterConfig& character_config,
	const elysia::io::CharacterAudioLayout& audio_layout,
	std::vector<SoundLoadRequest>& sound_load_requests
) const
{
	if (character_config.id.empty())
	{
		std::cout << "Build character audio requests failed: character id is empty." << std::endl;
		return false;
	}

	if (character_config.asset_key.empty())
	{
		std::cout << "Build character audio requests failed: asset key is empty: "
			<< character_config.id << std::endl;
		return false;
	}

	const std::filesystem::path audio_root = elysia::io::PathManager::instance()->audio() / "character" / character_config.asset_key;
	for (const elysia::io::CharacterAudioLayoutEntry& entry : audio_layout.sounds)
	{
		if (entry.key.empty())
		{
			std::cout << "Build character audio requests failed: sound key is empty." << std::endl;
			return false;
		}

		if (entry.path.empty())
		{
			std::cout << "Build character audio requests failed: sound path is empty: "
				<< entry.key << std::endl;
			return false;
		}

		const std::filesystem::path file_path = (audio_root / entry.path).lexically_normal();
		if (!std::filesystem::is_regular_file(file_path))
		{
			std::cout << "Build character audio requests failed: file does not exist: "
				<< file_path << std::endl;
			return false;
		}

		SoundLoadRequest request;
		request.key = character_config.id + "." + entry.key;
		request.file_path = file_path;
		sound_load_requests.push_back(std::move(request));
	}

	return true;
}
bool ResourceRequestBuilder::append_character_animation_requests(
	const elysia::io::CharacterConfig& character_config,
	const elysia::io::AnimationConfig& animation_config,
	std::vector<AtlasBuildRequest>& atlas_build_requests,
	std::vector<AnimationBuildRequest>& animation_build_requests
) const
{
	if (character_config.id.empty())
	{
		std::cout << "Build resource requests failed: character id is empty." << std::endl;
		return false;
	}

	for (const elysia::io::AnimationClipConfig& clip_config : animation_config.clips)
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

		AtlasBuildRequest atlas_request;
		atlas_request.atlas_key = animation_key;
		atlas_request.directory_path = std::move(directory_path);
		atlas_request.frame_count = clip_config.frame_count;

		AnimationBuildRequest animation_request;
		animation_request.animation_key = animation_key;
		animation_request.atlas_key = atlas_request.atlas_key;
		animation_request.fps = clip_config.fps;
		animation_request.loop = clip_config.loop;
		animation_request.segment_index = clip_config.segment_index;

		atlas_build_requests.push_back(std::move(atlas_request));
		animation_build_requests.push_back(std::move(animation_request));
	}

	return true;
}

bool ResourceRequestBuilder::append_character_effect_requests(
	const elysia::io::CharacterConfig& character_config,
	const elysia::io::AnimationConfig& animation_config,
	const elysia::io::CharacterEffectLayout& effect_layout,
	std::vector<AtlasBuildRequest>& atlas_build_requests,
	std::vector<AnimationBuildRequest>& animation_build_requests,
	std::vector<EffectBuildRequest>& effect_build_requests
) const
{
	if (character_config.id.empty())
	{
		std::cout << "Build effect requests failed: character id is empty." << std::endl;
		return false;
	}

	for (const elysia::io::AnimationClipConfig& clip_config : animation_config.clips)
	{
		std::unordered_map<std::string, elysia::io::CharacterEffectLayoutEntry>::const_iterator effect_iterator =
			effect_layout.effects.find(clip_config.animation_name);
		if (effect_iterator == effect_layout.effects.end())
			continue;

		if (clip_config.animation_name.empty())
		{
			std::cout << "Build effect requests failed: animation name is empty: "
				<< character_config.id << std::endl;
			return false;
		}

		const elysia::io::CharacterEffectLayoutEntry& effect_entry = effect_iterator->second;
		std::filesystem::path effect_relative_path;
		elysia::io::CharacterEffectPlaybackConfig playback_config;

		if (clip_config.is_segment)
		{
			if (!effect_entry.has_segment_path)
			{
				std::cout << "Build effect requests failed: segment_path is missing in effect config: "
					<< clip_config.animation_name << std::endl;
				return false;
			}

			if (clip_config.segment_index >= effect_entry.segments.size())
			{
				std::cout << "Skip effect requests: segment playback is missing: "
					<< character_config.id << "." << clip_config.animation_name
					<< "." << clip_config.segment_index << std::endl;
				continue;
			}

			effect_relative_path = resolve_segment_path(
				effect_entry.segment_path,
				clip_config.segment_index
			);
			playback_config = effect_entry.segments[clip_config.segment_index];
		}
		else
		{
			if (!effect_entry.has_path)
			{
				std::cout << "Build effect requests failed: path is missing in effect config: "
					<< clip_config.animation_name << std::endl;
				return false;
			}

			effect_relative_path = effect_entry.path;
			playback_config = effect_entry.playback;
		}

		std::filesystem::path directory_path =
			(character_config.texture_root / effect_relative_path).lexically_normal();
		if (directory_path.empty())
		{
			std::cout << "Build effect requests failed: effect directory is empty: "
				<< character_config.id << "." << clip_config.animation_name << std::endl;
			return false;
		}

		std::string effect_suffix = clip_config.animation_name;
		if (clip_config.is_segment)
			effect_suffix += "." + std::to_string(clip_config.segment_index);

		const std::string animation_key =
			character_config.id + ".effect." + effect_suffix;
		const std::string effect_key =
			character_config.id + ".effect." + effect_suffix;

		AtlasBuildRequest atlas_request;
		atlas_request.atlas_key = animation_key;
		atlas_request.directory_path = std::move(directory_path);
		atlas_request.frame_count = playback_config.frame_count;

		AnimationBuildRequest animation_request;
		animation_request.animation_key = animation_key;
		animation_request.atlas_key = atlas_request.atlas_key;
		animation_request.fps = playback_config.fps;
		animation_request.loop = playback_config.loop;
		animation_request.segment_index = clip_config.segment_index;

		EffectBuildRequest effect_request;
		effect_request.effect_key = effect_key;
		effect_request.animation_key = animation_key;

		atlas_build_requests.push_back(std::move(atlas_request));
		animation_build_requests.push_back(std::move(animation_request));
		effect_build_requests.push_back(std::move(effect_request));
	}

	return true;
}





}
