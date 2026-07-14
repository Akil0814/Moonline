#include "../../tools/logger.h"
#include "animation_layout_loader.h"

#include "../json/json_loader.h"

namespace elysia::io
{
bool AnimationLayoutLoader::load(const std::filesystem::path& layout_path, AnimationLayout& layout) const
{
	layout = AnimationLayout{};
	JsonLoader loader;
	if (!loader.open_file(layout_path) || !loader.root().is_object()
		|| !loader.root().contains("animations") || !loader.root().at("animations").is_object())
	{
		ELYSIA_LOG_WARN("io", "Load animation layout failed: animations is missing or invalid: " << layout_path);
		return false;
	}

	AnimationLayout parsed;
	for (json::const_iterator item = loader.root().at("animations").begin(); item != loader.root().at("animations").end(); ++item)
	{
		if (!item.value().is_object()) return false;
		const json& node = item.value();
		const bool has_path = node.contains("path");
		const bool has_segment_path = node.contains("segment_path");
		if (has_path == has_segment_path || (has_path && !node.at("path").is_string())
			|| (has_segment_path && !node.at("segment_path").is_string()))
		{
			ELYSIA_LOG_WARN("io", "Load animation layout failed: entry must contain exactly one path: " << item.key());
			return false;
		}
		AnimationLayoutEntry entry;
		entry.has_path = has_path;
		entry.has_segment_path = has_segment_path;
		if (has_path) entry.path = node.at("path").get<std::string>();
		else entry.segment_path = node.at("segment_path").get<std::string>();
		parsed.animations.emplace(item.key(), std::move(entry));
	}
	layout = std::move(parsed);
	return true;
}
}
