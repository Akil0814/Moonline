#include "animation_config_loader.h"

#include <iostream>
#include <utility>

bool AnimationConfigLoader::load(
	const std::filesystem::path& animation_config_path,
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
					parsed_config))
				{
					return false;
				}
			}

			continue;
		}

		if (!append_clip(animation.key(), false, 0, animation_node, parsed_config))
			return false;
	}

	config = std::move(parsed_config);
	return true;
}

bool AnimationConfigLoader::append_clip(
	const std::string& animation_name,
	bool is_segment,
	size_t segment_index,
	const json& clip_node,
	AnimationConfig& config
) const
{
	if (!clip_node.contains("path") || !clip_node.at("path").is_string())
	{
		std::cout << "Load animation clip failed: path is missing or not a string: "
			<< animation_name << std::endl;
		return false;
	}

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

	AnimationClipConfig clip_config;
	clip_config._animation_name = animation_name;
	clip_config._path = clip_node.at("path").get<std::string>();
	clip_config._frame_count = static_cast<size_t>(frame_count_value);
	clip_config._fps = fps;
	clip_config._loop = clip_node.at("loop").get<bool>();
	clip_config._is_segment = is_segment;
	clip_config._segment_index = segment_index;
	config._clips.push_back(std::move(clip_config));

	return true;
}
