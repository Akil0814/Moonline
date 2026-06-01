#include "animation_config_loader.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

bool AnimationConfigLoader::load(
	const std::filesystem::path& animation_config_path,
	const CharacterAnimationLayout& layout,
	AnimationConfig& config
) const
{
	config = AnimationConfig{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(animation_config_path);
	if (!result)
	{
		std::cout << "Load animation config failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load animation config failed: root is not an object: "
			<< animation_config_path << std::endl;
		return false;
	}

	AnimationConfig parsed_config;
	for (json::const_iterator animation = loader.root().begin();
		animation != loader.root().end();
		++animation)
	{
		if (!animation.value().is_object())
		{
			std::cout << "Load animation config failed: animation entry is not an object: "
				<< animation.key() << std::endl;
			return false;
		}

		const json& animation_node = animation.value();
		if (animation_node.contains("segments"))
		{
			if (!animation_node.at("segments").is_array())
			{
				std::cout << "Load animation config failed: segments is not an array: "
					<< animation.key() << std::endl;
				return false;
			}

			const json& segments = animation_node.at("segments");
			if (segments.empty())
			{
				std::cout << "Load animation config failed: segments is empty: "
					<< animation.key() << std::endl;
				return false;
			}

			for (size_t segment_index = 0; segment_index < segments.size(); ++segment_index)
			{
				if (!segments[segment_index].is_object())
				{
					std::cout << "Load animation config failed: segment is not an object: "
						<< animation.key() << std::endl;
					return false;
				}

				if (!append_clip(
					animation.key(),
					true,
					segment_index,
					segments[segment_index],
					layout,
					parsed_config))
				{
					return false;
				}
			}

			continue;
		}

		if (!append_clip(animation.key(), false, 0, animation_node, layout, parsed_config))
			return false;
	}

	config = std::move(parsed_config);
	return true;
}

std::filesystem::path AnimationConfigLoader::resolve_clip_path(
	const std::string& animation_name,
	bool is_segment,
	size_t segment_index,
	const json& clip_node,
	const CharacterAnimationLayout& layout
) const
{
	if (clip_node.contains("override_path"))
	{
		if (!clip_node.at("override_path").is_string())
		{
			std::cout << "Load animation clip failed: override_path is not a string: "
				<< animation_name << std::endl;
			return {};
		}

		return clip_node.at("override_path").get<std::string>();
	}

	std::unordered_map<std::string, CharacterAnimationLayoutEntry>::const_iterator iterator =
		layout._animations.find(animation_name);
	if (iterator == layout._animations.end())
	{
		std::cout << "Load animation clip failed: layout entry does not exist: "
			<< animation_name << std::endl;
		return {};
	}

	const CharacterAnimationLayoutEntry& entry = iterator->second;
	if (is_segment)
	{
		if (!entry._has_segment_path)
		{
			std::cout << "Load animation clip failed: segment_path is missing in layout: "
				<< animation_name << std::endl;
			return {};
		}

		return resolve_segment_path(entry._segment_path, segment_index);
	}

	if (!entry._has_path)
	{
		std::cout << "Load animation clip failed: path is missing in layout: "
			<< animation_name << std::endl;
		return {};
	}

	return entry._path;
}

std::filesystem::path AnimationConfigLoader::resolve_segment_path(
	const std::filesystem::path& segment_path,
	size_t segment_index
) const
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

bool AnimationConfigLoader::append_clip(
	const std::string& animation_name,
	bool is_segment,
	size_t segment_index,
	const json& clip_node,
	const CharacterAnimationLayout& layout,
	AnimationConfig& config
) const
{
	if (!clip_node.contains("frame_count") || !clip_node.at("frame_count").is_number_integer())
	{
		std::cout << "Load animation clip failed: frame_count is missing or invalid: "
			<< animation_name << std::endl;
		return false;
	}

	if (!clip_node.contains("fps") || !clip_node.at("fps").is_number())
	{
		std::cout << "Load animation clip failed: fps is missing or invalid: "
			<< animation_name << std::endl;
		return false;
	}

	if (!clip_node.contains("loop") || !clip_node.at("loop").is_boolean())
	{
		std::cout << "Load animation clip failed: loop is missing or invalid: "
			<< animation_name << std::endl;
		return false;
	}

	const int frame_count_value = clip_node.at("frame_count").get<int>();
	const double fps = clip_node.at("fps").get<double>();
	if (frame_count_value <= 0 || fps <= 0.0)
	{
		std::cout << "Load animation clip failed: frame_count or fps is invalid: "
			<< animation_name << std::endl;
		return false;
	}

	std::filesystem::path clip_path =
		resolve_clip_path(animation_name, is_segment, segment_index, clip_node, layout);
	if (clip_path.empty())
		return false;

	AnimationClipConfig clip_config;
	clip_config._animation_name = animation_name;
	clip_config._path = clip_path;
	clip_config._frame_count = static_cast<size_t>(frame_count_value);
	clip_config._fps = fps;
	clip_config._loop = clip_node.at("loop").get<bool>();
	clip_config._is_segment = is_segment;
	clip_config._segment_index = segment_index;
	config._clips.push_back(std::move(clip_config));

	return true;
}
