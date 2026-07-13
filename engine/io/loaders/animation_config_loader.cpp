#include "../../tools/logger.h"
#include "animation_config_loader.h"
#include <string>
#include <unordered_map>
#include <utility>

namespace elysia::io
{
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
		ELYSIA_LOG_WARN("io","Load animation config failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_WARN("io","Load animation config failed: root is not an object: "
			<< animation_config_path);
		return false;
	}

	AnimationConfig parsed_config;
	for (json::const_iterator animation = loader.root().begin();
		animation != loader.root().end();
		++animation)
	{
		if (!animation.value().is_object())
		{
			ELYSIA_LOG_WARN("io","Load animation config failed: animation entry is not an object: "
				<< animation.key());
			return false;
		}

		const json& animation_node = animation.value();
		if (animation_node.contains("segments"))
		{
			if (!animation_node.at("segments").is_array())
			{
				ELYSIA_LOG_WARN("io","Load animation config failed: segments is not an array: "
					<< animation.key());
				return false;
			}

			const json& segments = animation_node.at("segments");
			if (segments.empty())
			{
				ELYSIA_LOG_WARN("io","Load animation config failed: segments is empty: "
					<< animation.key());
				return false;
			}

			for (size_t segment_index = 0; segment_index < segments.size(); ++segment_index)
			{
				if (!segments[segment_index].is_object())
				{
					ELYSIA_LOG_WARN("io","Load animation config failed: segment is not an object: "
						<< animation.key());
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
			ELYSIA_LOG_WARN("io","Load animation clip failed: override_path is not a string: "
				<< animation_name);
			return {};
		}

		return clip_node.at("override_path").get<std::string>();
	}

	std::unordered_map<std::string, CharacterAnimationLayoutEntry>::const_iterator iterator =
		layout.animations.find(animation_name);
	if (iterator == layout.animations.end())
	{
		ELYSIA_LOG_WARN("io","Load animation clip failed: layout entry does not exist: "
			<< animation_name);
		return {};
	}

	const CharacterAnimationLayoutEntry& entry = iterator->second;
	if (is_segment)
	{
		if (!entry.has_segment_path)
		{
			ELYSIA_LOG_WARN("io","Load animation clip failed: segment_path is missing in layout: "
				<< animation_name);
			return {};
		}

		return resolve_segment_path(entry.segment_path, segment_index);
	}

	if (!entry.has_path)
	{
		ELYSIA_LOG_WARN("io","Load animation clip failed: path is missing in layout: "
			<< animation_name);
		return {};
	}

	return entry.path;
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
		ELYSIA_LOG_WARN("io","Load animation clip failed: frame_count is missing or invalid: "
			<< animation_name);
		return false;
	}

	if (!clip_node.contains("fps") || !clip_node.at("fps").is_number())
	{
		ELYSIA_LOG_WARN("io","Load animation clip failed: fps is missing or invalid: "
			<< animation_name);
		return false;
	}

	if (!clip_node.contains("loop") || !clip_node.at("loop").is_boolean())
	{
		ELYSIA_LOG_WARN("io","Load animation clip failed: loop is missing or invalid: "
			<< animation_name);
		return false;
	}

	const int frame_count_value = clip_node.at("frame_count").get<int>();
	const double fps = clip_node.at("fps").get<double>();
	if (frame_count_value <= 0 || fps <= 0.0)
	{
		ELYSIA_LOG_WARN("io","Load animation clip failed: frame_count or fps is invalid: "
			<< animation_name);
		return false;
	}

	std::filesystem::path clip_path =
		resolve_clip_path(animation_name, is_segment, segment_index, clip_node, layout);
	if (clip_path.empty())
		return false;

	AnimationClipConfig clip_config;
	clip_config.animation_name = animation_name;
	clip_config.path = clip_path;
	clip_config.frame_count = static_cast<size_t>(frame_count_value);
	clip_config.fps = fps;
	clip_config.loop = clip_node.at("loop").get<bool>();
	clip_config.is_segment = is_segment;
	clip_config.segment_index = segment_index;
	config.clips.push_back(std::move(clip_config));

	return true;
}

}
