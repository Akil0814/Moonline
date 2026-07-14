#include "../../tools/logger.h"
#include "animation_effect_layout_loader.h"
#include "../json/json_loader.h"

namespace elysia::io
{
namespace
{
bool parse_playback(const json& node, const std::string& key, AnimationEffectPlaybackConfig& out)
{
	if (!node.contains("frame_count") || !node.at("frame_count").is_number_unsigned()
		|| !node.contains("fps") || !node.at("fps").is_number()
		|| !node.contains("loop") || !node.at("loop").is_boolean())
	{
		ELYSIA_LOG_WARN("io", "Load animation effect layout failed: invalid playback: " << key);
		return false;
	}
	out.frame_count = node.at("frame_count").get<size_t>();
	out.fps = node.at("fps").get<double>();
	out.loop = node.at("loop").get<bool>();
	return out.frame_count > 0 && out.fps > 0.0;
}
}

bool AnimationEffectLayoutLoader::load(const std::filesystem::path& path, AnimationEffectLayout& layout) const
{
	layout = {};
	JsonLoader loader;
	if (!loader.open_file(path) || !loader.root().is_object()
		|| !loader.root().contains("effects") || !loader.root().at("effects").is_object())
	{
		ELYSIA_LOG_WARN("io", "Load animation effect layout failed: effects is missing or invalid: " << path);
		return false;
	}
	for (auto it = loader.root().at("effects").begin(); it != loader.root().at("effects").end(); ++it)
	{
		if (!it.value().is_object()) return false;
		const json& node = it.value();
		AnimationEffectLayoutEntry entry;
		if (node.contains("path"))
		{
			if (!node.at("path").is_string() || node.contains("segment_path") || node.contains("segments")
				|| !parse_playback(node, it.key(), entry.playback)) return false;
			entry.path = node.at("path").get<std::string>(); entry.has_path = true;
		}
		else if (node.contains("segment_path"))
		{
			if (!node.at("segment_path").is_string() || !node.contains("segments") || !node.at("segments").is_array()
				|| node.at("segments").empty() || node.contains("frame_count") || node.contains("fps") || node.contains("loop")) return false;
			entry.segment_path = node.at("segment_path").get<std::string>(); entry.has_segment_path = true;
			for (const json& segment : node.at("segments")) { AnimationEffectPlaybackConfig playback; if (!segment.is_object() || !parse_playback(segment, it.key(), playback)) return false; entry.segments.push_back(playback); }
		}
		else return false;
		layout.effects.emplace(it.key(), std::move(entry));
	}
	return true;
}
}
