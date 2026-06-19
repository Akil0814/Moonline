#include "character_effect_layout_loader.h"

#include "../json/json_loader.h"

#include <iostream>
#include <utility>

namespace
{
bool parse_playback_config(
	const json& playback_node,
	const std::string& effect_key,
	const char* error_prefix,
	CharacterEffectPlaybackConfig& out_config
)
{
	if (!playback_node.is_object())
	{
		std::cout << error_prefix << ": playback entry is not an object: "
			<< effect_key << std::endl;
		return false;
	}

	if (!playback_node.contains("frame_count")
		|| !playback_node.at("frame_count").is_number_unsigned())
	{
		std::cout << error_prefix << ": frame_count is missing or invalid: "
			<< effect_key << std::endl;
		return false;
	}

	if (!playback_node.contains("fps")
		|| !playback_node.at("fps").is_number())
	{
		std::cout << error_prefix << ": fps is missing or invalid: "
			<< effect_key << std::endl;
		return false;
	}

	if (!playback_node.contains("loop")
		|| !playback_node.at("loop").is_boolean())
	{
		std::cout << error_prefix << ": loop is missing or invalid: "
			<< effect_key << std::endl;
		return false;
	}

	out_config.frame_count = playback_node.at("frame_count").get<size_t>();
	out_config.fps = playback_node.at("fps").get<double>();
	out_config.loop = playback_node.at("loop").get<bool>();

	if (out_config.frame_count == 0)
	{
		std::cout << error_prefix << ": frame_count must be positive: "
			<< effect_key << std::endl;
		return false;
	}

	if (out_config.fps <= 0.0)
	{
		std::cout << error_prefix << ": fps must be positive: "
			<< effect_key << std::endl;
		return false;
	}

	return true;
}
}

bool CharacterEffectLayoutLoader::load(
	const std::filesystem::path& layout_path,
	CharacterEffectLayout& layout
) const
{
	layout = CharacterEffectLayout{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(layout_path);
	if (!result)
	{
		std::cout << "Load character effect layout failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character effect layout failed: root is not an object: "
			<< layout_path << std::endl;
		return false;
	}

	if (!loader.root().contains("effects") || !loader.root().at("effects").is_object())
	{
		std::cout << "Load character effect layout failed: effects is missing or not an object: "
			<< layout_path << std::endl;
		return false;
	}

	CharacterEffectLayout parsed_layout;
	const json& effects = loader.root().at("effects");
	for (json::const_iterator effect = effects.begin();
		effect != effects.end();
		++effect)
	{
		if (!effect.value().is_object())
		{
			std::cout << "Load character effect layout failed: effect entry is not an object: "
				<< effect.key() << std::endl;
			return false;
		}

		const json& effect_node = effect.value();
		const bool has_path = effect_node.contains("path");
		const bool has_segment_path = effect_node.contains("segment_path");
		const bool has_segments = effect_node.contains("segments");

		CharacterEffectLayoutEntry entry;
		if (has_path)
		{
			if (has_segment_path || has_segments)
			{
				std::cout << "Load character effect layout failed: mixed fixed and segmented effect schema: "
					<< effect.key() << std::endl;
				return false;
			}

			if (!effect_node.at("path").is_string())
			{
				std::cout << "Load character effect layout failed: path is not a string: "
					<< effect.key() << std::endl;
				return false;
			}

			entry.path = effect_node.at("path").get<std::string>();
			entry.has_path = true;
			if (!parse_playback_config(
				effect_node,
				effect.key(),
				"Load character effect layout failed",
				entry.playback))
			{
				return false;
			}
		}
		else if (has_segment_path)
		{
			if (!effect_node.at("segment_path").is_string())
			{
				std::cout << "Load character effect layout failed: segment_path is not a string: "
					<< effect.key() << std::endl;
				return false;
			}

			if (!has_segments || !effect_node.at("segments").is_array())
			{
				std::cout << "Load character effect layout failed: segments is missing or not an array: "
					<< effect.key() << std::endl;
				return false;
			}

			if (effect_node.contains("frame_count")
				|| effect_node.contains("fps")
				|| effect_node.contains("loop")
				|| effect_node.contains("path"))
			{
				std::cout << "Load character effect layout failed: segmented entry contains fixed playback fields: "
					<< effect.key() << std::endl;
				return false;
			}

			entry.segment_path = effect_node.at("segment_path").get<std::string>();
			entry.has_segment_path = true;

			const json& segments = effect_node.at("segments");
			if (segments.empty())
			{
				std::cout << "Load character effect layout failed: segments is empty: "
					<< effect.key() << std::endl;
				return false;
			}

			entry.segments.reserve(segments.size());
			for (size_t index = 0; index < segments.size(); ++index)
			{
				CharacterEffectPlaybackConfig segment_config;
				const std::string segment_key = effect.key() + "." + std::to_string(index);
				if (!parse_playback_config(
					segments.at(index),
					segment_key,
					"Load character effect layout failed",
					segment_config))
				{
					return false;
				}

				entry.segments.push_back(std::move(segment_config));
			}
		}
		else
		{
			std::cout << "Load character effect layout failed: path or segment_path is missing: "
				<< effect.key() << std::endl;
			return false;
		}

		parsed_layout.effects.emplace(effect.key(), std::move(entry));
	}

	layout = std::move(parsed_layout);
	return true;
}
