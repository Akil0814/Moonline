#include "../../tools/logger.h"
#include "character_effect_layout_loader.h"

#include "../json/json_loader.h"
#include <utility>

namespace elysia::io
{
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
		ELYSIA_LOG_WARN("io",error_prefix << ": playback entry is not an object: "
			<< effect_key);
		return false;
	}

	if (!playback_node.contains("frame_count")
		|| !playback_node.at("frame_count").is_number_unsigned())
	{
		ELYSIA_LOG_WARN("io",error_prefix << ": frame_count is missing or invalid: "
			<< effect_key);
		return false;
	}

	if (!playback_node.contains("fps")
		|| !playback_node.at("fps").is_number())
	{
		ELYSIA_LOG_WARN("io",error_prefix << ": fps is missing or invalid: "
			<< effect_key);
		return false;
	}

	if (!playback_node.contains("loop")
		|| !playback_node.at("loop").is_boolean())
	{
		ELYSIA_LOG_WARN("io",error_prefix << ": loop is missing or invalid: "
			<< effect_key);
		return false;
	}

	out_config.frame_count = playback_node.at("frame_count").get<size_t>();
	out_config.fps = playback_node.at("fps").get<double>();
	out_config.loop = playback_node.at("loop").get<bool>();

	if (out_config.frame_count == 0)
	{
		ELYSIA_LOG_WARN("io",error_prefix << ": frame_count must be positive: "
			<< effect_key);
		return false;
	}

	if (out_config.fps <= 0.0)
	{
		ELYSIA_LOG_WARN("io",error_prefix << ": fps must be positive: "
			<< effect_key);
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
		ELYSIA_LOG_WARN("io","Load character effect layout failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_WARN("io","Load character effect layout failed: root is not an object: "
			<< layout_path);
		return false;
	}

	if (!loader.root().contains("effects") || !loader.root().at("effects").is_object())
	{
		ELYSIA_LOG_WARN("io","Load character effect layout failed: effects is missing or not an object: "
			<< layout_path);
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
			ELYSIA_LOG_WARN("io","Load character effect layout failed: effect entry is not an object: "
				<< effect.key());
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
				ELYSIA_LOG_WARN("io","Load character effect layout failed: mixed fixed and segmented effect schema: "
					<< effect.key());
				return false;
			}

			if (!effect_node.at("path").is_string())
			{
				ELYSIA_LOG_WARN("io","Load character effect layout failed: path is not a string: "
					<< effect.key());
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
				ELYSIA_LOG_WARN("io","Load character effect layout failed: segment_path is not a string: "
					<< effect.key());
				return false;
			}

			if (!has_segments || !effect_node.at("segments").is_array())
			{
				ELYSIA_LOG_WARN("io","Load character effect layout failed: segments is missing or not an array: "
					<< effect.key());
				return false;
			}

			if (effect_node.contains("frame_count")
				|| effect_node.contains("fps")
				|| effect_node.contains("loop")
				|| effect_node.contains("path"))
			{
				ELYSIA_LOG_WARN("io","Load character effect layout failed: segmented entry contains fixed playback fields: "
					<< effect.key());
				return false;
			}

			entry.segment_path = effect_node.at("segment_path").get<std::string>();
			entry.has_segment_path = true;

			const json& segments = effect_node.at("segments");
			if (segments.empty())
			{
				ELYSIA_LOG_WARN("io","Load character effect layout failed: segments is empty: "
					<< effect.key());
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
			ELYSIA_LOG_WARN("io","Load character effect layout failed: path or segment_path is missing: "
				<< effect.key());
			return false;
		}

		parsed_layout.effects.emplace(effect.key(), std::move(entry));
	}

	layout = std::move(parsed_layout);
	return true;
}

}
