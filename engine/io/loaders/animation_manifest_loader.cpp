#include "../../tools/logger.h"
#include "animation_manifest_loader.h"

#include "../json/json_loader.h"
#include <unordered_set>
#include <utility>

namespace elysia::io
{
bool AnimationManifestLoader::load(
	const std::filesystem::path& manifest_path,
	AnimationManifest& manifest
) const
{
	manifest = AnimationManifest{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(manifest_path);
	if (!result)
	{
		ELYSIA_LOG_WARN("io","Load animation manifest failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object()
		|| !loader.root().contains("animations")
		|| !loader.root().at("animations").is_array())
	{
		ELYSIA_LOG_WARN("io","Load animation manifest failed: animations is missing or not an array: "
			<< manifest_path);
		return false;
	}

	AnimationManifest parsed_manifest;
	std::unordered_set<std::string> keys;
	for (const json& animation_node : loader.root().at("animations"))
	{
		if (!animation_node.is_object()
			|| !animation_node.contains("key") || !animation_node.at("key").is_string()
			|| !animation_node.contains("path") || !animation_node.at("path").is_string()
			|| !animation_node.contains("frame_count") || !animation_node.at("frame_count").is_number_integer()
			|| !animation_node.contains("fps") || !animation_node.at("fps").is_number()
			|| !animation_node.contains("loop") || !animation_node.at("loop").is_boolean())
		{
			ELYSIA_LOG_WARN("io","Load animation manifest failed: invalid animation entry: "
				<< manifest_path);
			return false;
		}

		AnimationManifestEntry entry;
		entry.key = animation_node.at("key").get<std::string>();
		entry.directory_path = animation_node.at("path").get<std::string>();
		entry.frame_count = animation_node.at("frame_count").get<size_t>();
		entry.fps = animation_node.at("fps").get<double>();
		entry.loop = animation_node.at("loop").get<bool>();

		if (entry.key.empty() || entry.directory_path.empty() || entry.frame_count == 0 || entry.fps <= 0.0)
		{
			ELYSIA_LOG_WARN("io","Load animation manifest failed: invalid animation values: "
				<< manifest_path);
			return false;
		}

		if (!keys.insert(entry.key).second)
		{
			ELYSIA_LOG_WARN("io","Load animation manifest failed: duplicate animation key: "
				<< entry.key);
			return false;
		}

		parsed_manifest.animations.push_back(std::move(entry));
	}

	manifest = std::move(parsed_manifest);
	return true;
}
}
