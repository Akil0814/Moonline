#include "../../tools/logger.h"
#include "effect_manifest_loader.h"

#include "../json/json_loader.h"

#include <unordered_set>
#include <utility>

namespace elysia::io
{
bool EffectManifestLoader::load(
	const std::filesystem::path& manifest_path,
	EffectManifest& manifest
) const
{
	manifest = EffectManifest{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(manifest_path);
	if (!result)
	{
		ELYSIA_LOG_WARN("io","Load effect manifest failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object()
		|| !loader.root().contains("effects")
		|| !loader.root().at("effects").is_array())
	{
		ELYSIA_LOG_WARN("io","Load effect manifest failed: effects is missing or not an array: "
			<< manifest_path);
		return false;
	}

	EffectManifest parsed_manifest;
	std::unordered_set<std::string> keys;
	for (const json& effect_node : loader.root().at("effects"))
	{
		if (!effect_node.is_object()
			|| !effect_node.contains("key") || !effect_node.at("key").is_string()
			|| !effect_node.contains("animation_key") || !effect_node.at("animation_key").is_string())
		{
			ELYSIA_LOG_WARN("io","Load effect manifest failed: invalid effect entry: "
				<< manifest_path);
			return false;
		}

		EffectManifestEntry entry;
		entry.key = effect_node.at("key").get<std::string>();
		entry.animation_key = effect_node.at("animation_key").get<std::string>();
		if (entry.key.empty() || entry.animation_key.empty())
		{
			ELYSIA_LOG_WARN("io","Load effect manifest failed: empty effect values: "
				<< manifest_path);
			return false;
		}

		if (!keys.insert(entry.key).second)
		{
			ELYSIA_LOG_WARN("io","Load effect manifest failed: duplicate effect key: "
				<< entry.key);
			return false;
		}

		parsed_manifest.effects.push_back(std::move(entry));
	}

	manifest = std::move(parsed_manifest);
	return true;
}
}
