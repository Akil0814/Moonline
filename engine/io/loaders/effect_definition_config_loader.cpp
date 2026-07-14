#include "../../tools/logger.h"
#include "effect_definition_config_loader.h"

#include "../json/json_loader.h"

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace elysia::io
{
namespace
{
bool has_duplicate_object_key(const std::filesystem::path& path)
{
	std::ifstream input(path);
	if (!input.is_open()) return false;
	std::unordered_map<int, std::unordered_set<std::string>> keys_by_depth;
	bool duplicate = false;
	const json::parser_callback_t callback = [&keys_by_depth, &duplicate](
		int depth, json::parse_event_t event, json& parsed)
	{
		if (event == json::parse_event_t::object_start)
			keys_by_depth[depth + 1].clear();
		else if (event == json::parse_event_t::key)
			duplicate = duplicate || !keys_by_depth[depth].insert(parsed.get<std::string>()).second;
		else if (event == json::parse_event_t::object_end)
			keys_by_depth.erase(depth + 1);
		return true;
	};
	try
	{
		[[maybe_unused]] const json parsed = json::parse(input, callback);
	}
	catch (const std::exception&)
	{
		return false;
	}
	return duplicate;
}
}

bool EffectDefinitionConfigLoader::load(
	const std::filesystem::path& config_path,
	const AnimationConfig& animation_config,
	EffectDefinitionConfig& config
) const
{
	config = {};
	const auto fail = [&config_path](const std::string& message)
	{
		ELYSIA_LOG_WARN("io", "Load effect definition config failed: " << message << ": " << config_path);
		return false;
	};
	if (has_duplicate_object_key(config_path))
		return fail("duplicate object key");
	JsonLoader loader;
	if (!loader.open_file(config_path) || !loader.root().is_object()
		|| !loader.root().contains("effects") || !loader.root().at("effects").is_object())
	{
		ELYSIA_LOG_WARN("io", "Load effect definition config failed: effects is missing or invalid: " << config_path);
		return false;
	}

	for (auto root_field = loader.root().begin(); root_field != loader.root().end(); ++root_field)
		if (root_field.key() != "effects")
		{
			ELYSIA_LOG_WARN("io", "Load effect definition config failed: unknown root field: " << root_field.key());
			return false;
		}

	EffectDefinitionConfig parsed;
	std::unordered_set<std::string> effect_names;
	for (auto item = loader.root().at("effects").begin(); item != loader.root().at("effects").end(); ++item)
	{
		if (item.key().empty() || !effect_names.insert(item.key()).second || !item.value().is_object())
		{
			ELYSIA_LOG_WARN("io", "Load effect definition config failed: invalid or duplicate effect: " << item.key());
			return false;
		}
		const json& node = item.value();
		for (auto field = node.begin(); field != node.end(); ++field)
			if (field.key() != "animation" && field.key() != "default_width"
				&& field.key() != "default_height" && field.key() != "default_angle_degrees")
			{
				ELYSIA_LOG_WARN("io", "Load effect definition config failed: unknown effect field: " << field.key());
				return false;
			}
		if (!node.contains("animation") || !node.at("animation").is_string())
			return fail("animation is missing or invalid for " + item.key());

		EffectDefinitionConfigEntry entry;
		entry.effect_name = item.key();
		entry.animation_name = node.at("animation").get<std::string>();
		if (entry.animation_name.empty()) return fail("animation is empty for " + item.key());
		if (node.contains("default_width"))
		{
			if (!node.at("default_width").is_number()) return fail("default_width is invalid for " + item.key());
			entry.default_width = node.at("default_width").get<float>();
		}
		if (node.contains("default_height"))
		{
			if (!node.at("default_height").is_number()) return fail("default_height is invalid for " + item.key());
			entry.default_height = node.at("default_height").get<float>();
		}
		if (node.contains("default_angle_degrees"))
		{
			if (!node.at("default_angle_degrees").is_number()) return fail("default_angle_degrees is invalid for " + item.key());
			entry.default_angle_degrees = node.at("default_angle_degrees").get<double>();
		}
		if (entry.default_width < 0.0f || entry.default_height < 0.0f
			|| ((entry.default_width == 0.0f) != (entry.default_height == 0.0f)))
		{
			ELYSIA_LOG_WARN("io", "Load effect definition config failed: default width and height must both be zero or positive: " << item.key());
			return false;
		}
		const bool animation_exists = std::any_of(
			animation_config.clips.begin(), animation_config.clips.end(),
			[&entry](const AnimationClipConfig& clip) { return clip.animation_name == entry.animation_name; });
		if (!animation_exists)
		{
			ELYSIA_LOG_WARN("io", "Load effect definition config failed: animation does not exist: " << entry.animation_name);
			return false;
		}
		parsed.effects.push_back(std::move(entry));
	}

	config = std::move(parsed);
	return true;
}
}
