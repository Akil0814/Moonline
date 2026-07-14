#include "animated_entity_content_loader.h"

#include "../io/loaders/animation_config_loader.h"
#include "../io/loaders/animation_layout_loader.h"
#include "../io/loaders/effect_definition_config_loader.h"
#include "../io/loaders/entity_audio_layout_loader.h"
#include "../io/loaders/entity_manifest_loader.h"
#include "../io/loaders/entity_texture_layout_loader.h"
#include "../io/json/json_loader.h"
#include "../io/path/path_manager.h"

#include <string_view>
#include <unordered_map>

namespace elysia::loading
{
namespace
{
bool read_path(const elysia::io::json& node, std::string_view key, std::filesystem::path& out, std::string& error)
{
	if (!node.contains(std::string(key)) || !node.at(std::string(key)).is_string())
	{
		error = "Animated entity content failed: missing or invalid " + std::string(key) + ".";
		return false;
	}
	out = elysia::io::PathManager::instance()->to_asset_path(node.at(std::string(key)).get<std::string>());
	return true;
}

bool resolve_template(
	const std::filesystem::path& pattern,
	std::string_view field_name,
	const std::string& asset_key,
	std::filesystem::path& out,
	std::string& error)
{
	std::string value = pattern.string();
	const size_t marker = value.find("{asset_key}");
	if (marker == std::string::npos)
	{
		error = "Animated entity content failed: " + std::string(field_name) + " must contain {asset_key}.";
		return false;
	}
	value.replace(marker, std::string("{asset_key}").size(), asset_key);
	out = elysia::io::PathManager::instance()->to_config_path(value);
	if (!std::filesystem::is_regular_file(out))
	{
		error = "Animated entity content failed: configured file does not exist: " + out.string();
		return false;
	}
	return true;
}

bool resolve_entity_texture_root(const std::string& pattern, const std::string& asset_key,
	std::filesystem::path& out, std::string& error)
{
	std::string value = pattern;
	const size_t marker = value.find("{asset_key}");
	if (marker == std::string::npos)
		value = (std::filesystem::path(value) / asset_key).generic_string();
	else
		value.replace(marker, std::string("{asset_key}").size(), asset_key);
	out = elysia::io::PathManager::instance()->to_asset_path(value);
	if (!std::filesystem::is_directory(out))
	{
		error = "Animated entity content failed: entity texture root does not exist: " + out.string();
		return false;
	}
	return true;
}

bool load_layouts(const elysia::io::json& capability, std::unordered_map<std::string, elysia::io::AnimationLayout>& layouts,
	std::string_view label, std::string& error)
{
	if (!capability.contains("layouts") || !capability.at("layouts").is_object())
	{
		error = "Animated entity content failed: " + std::string(label) + " requires layouts.";
		return false;
	}
	elysia::io::AnimationLayoutLoader loader;
	for (auto item = capability.at("layouts").begin(); item != capability.at("layouts").end(); ++item)
	{
		if (!item.value().is_string())
		{
			error = "Animated entity content failed: " + std::string(label) + " layout path is invalid.";
			return false;
		}
		const auto path = elysia::io::PathManager::instance()->to_asset_path(item.value().get<std::string>());
		elysia::io::AnimationLayout layout;
		if (!std::filesystem::is_regular_file(path) || !loader.load(path, layout))
		{
			error = "Animated entity content failed: " + std::string(label) + " layout load failed.";
			return false;
		}
		layouts.emplace(item.key(), std::move(layout));
	}
	return true;
}
}

bool AnimatedEntityContentLoader::load(const std::filesystem::path& path, elysia::io::AnimatedEntityContent& content, std::string& error) const
{
	content = {};
	elysia::io::JsonLoader json_loader;
	if (!json_loader.open_file(path) || !json_loader.root().is_object())
	{
		error = "Animated entity content failed: module manifest is invalid.";
		return false;
	}
	const elysia::io::json& root = json_loader.root();
	for (auto item = root.begin(); item != root.end(); ++item)
		if (item.key() != "entities" && item.key() != "resources" && item.key() != "capabilities")
		{
			error = "Animated entity content failed: unknown manifest key: " + item.key();
			return false;
		}

	std::filesystem::path entities_path;
	if (!read_path(root, "entities", entities_path, error) || !std::filesystem::is_regular_file(entities_path))
	{
		if (error.empty()) error = "Animated entity content failed: entities manifest does not exist.";
		return false;
	}
	elysia::io::EntityManifest entity_manifest;
	elysia::io::EntityManifestLoader entity_loader;
	if (!entity_loader.load(entities_path, entity_manifest))
	{
		error = "Animated entity content failed: entity manifest load failed.";
		return false;
	}

	const elysia::io::json* resources = nullptr;
	if (root.contains("resources"))
	{
		if (!root.at("resources").is_object()) { error = "Animated entity content failed: resources is not an object."; return false; }
		resources = &root.at("resources");
		for (auto item = resources->begin(); item != resources->end(); ++item)
			if (item.key() != "texture_root" && item.key() != "animation_config_template" && item.key() != "audio_root"
				&& item.key() != "effect_animation_config_template" && item.key() != "effect_info_template")
			{
				error = "Animated entity content failed: unknown resource field: " + item.key();
				return false;
			}
	}
	const elysia::io::json* capabilities = nullptr;
	if (root.contains("capabilities"))
	{
		if (!root.at("capabilities").is_object()) { error = "Animated entity content failed: capabilities is not an object."; return false; }
		capabilities = &root.at("capabilities");
	}
	if (!capabilities)
	{
		for (const auto& entity : entity_manifest.entities)
			content.entities.push_back({entity.id, entity.asset_key, {}, {}, entity.horizontal_strip});
		return true;
	}
	for (auto item = capabilities->begin(); item != capabilities->end(); ++item)
	{
		if (item.key() != "animations" && item.key() != "textures" && item.key() != "audio" && item.key() != "effects")
		{
			error = "Animated entity content failed: unknown capability: " + item.key(); return false;
		}
		if (!item.value().is_object()) { error = "Animated entity content failed: capability is not an object: " + item.key(); return false; }
		const std::string expected = (item.key() == "animations" || item.key() == "effects") ? "layouts" : "layout";
		for (auto field = item.value().begin(); field != item.value().end(); ++field)
			if (field.key() != expected) { error = "Animated entity content failed: unknown capability field: " + field.key(); return false; }
	}

	std::string texture_root_template;
	std::filesystem::path animation_template, effect_animation_template, effect_info_template, audio_root;
	if (resources)
	{
		for (const char* key : {"texture_root", "animation_config_template", "effect_animation_config_template", "effect_info_template"})
			if (resources->contains(key) && !resources->at(key).is_string()) { error = "Animated entity content failed: invalid " + std::string(key) + "."; return false; }
		if (resources->contains("texture_root")) texture_root_template = resources->at("texture_root").get<std::string>();
		if (resources->contains("animation_config_template")) animation_template = resources->at("animation_config_template").get<std::string>();
		if (resources->contains("effect_animation_config_template")) effect_animation_template = resources->at("effect_animation_config_template").get<std::string>();
		if (resources->contains("effect_info_template")) effect_info_template = resources->at("effect_info_template").get<std::string>();
		if (resources->contains("audio_root") && !read_path(*resources, "audio_root", audio_root, error)) return false;
	}
	const bool has_animations = capabilities->contains("animations");
	const bool has_effects = capabilities->contains("effects");
	if ((has_animations || has_effects || capabilities->contains("textures")) && texture_root_template.empty())
	{
		error = "Animated entity content failed: texture_root is required."; return false;
	}
	if (has_animations && animation_template.empty()) { error = "Animated entity content failed: animation_config_template is required."; return false; }
	if (has_effects && (effect_animation_template.empty() || effect_info_template.empty()))
	{
		error = "Animated entity content failed: effects requires effect animation and effect info templates."; return false;
	}
	if (capabilities->contains("audio") && (audio_root.empty() || !std::filesystem::is_directory(audio_root)))
	{
		error = "Animated entity content failed: audio_root is required and must exist."; return false;
	}

	for (const auto& entity : entity_manifest.entities)
	{
		std::filesystem::path entity_texture_root;
		if ((has_animations || has_effects || capabilities->contains("textures"))
			&& !resolve_entity_texture_root(texture_root_template, entity.asset_key, entity_texture_root, error)) return false;
		content.entities.push_back({entity.id, entity.asset_key, std::move(entity_texture_root), audio_root, entity.horizontal_strip});
	}

	if (capabilities->contains("textures"))
	{
		std::filesystem::path layout_path;
		if (!read_path(capabilities->at("textures"), "layout", layout_path, error) || !std::filesystem::is_regular_file(layout_path)) return false;
		elysia::io::EntityTextureLayout layout; elysia::io::EntityTextureLayoutLoader loader;
		if (!loader.load(layout_path, layout)) { error = "Animated entity content failed: texture layout load failed."; return false; }
		content.texture_layout = std::move(layout);
	}
	if (capabilities->contains("audio"))
	{
		std::filesystem::path layout_path;
		if (!read_path(capabilities->at("audio"), "layout", layout_path, error) || !std::filesystem::is_regular_file(layout_path)) return false;
		elysia::io::EntityAudioLayout layout; elysia::io::EntityAudioLayoutLoader loader;
		if (!loader.load(layout_path, layout)) { error = "Animated entity content failed: audio layout load failed."; return false; }
		content.audio_layout = std::move(layout);
	}

	std::unordered_map<std::string, elysia::io::AnimationLayout> animation_layouts, effect_layouts;
	if (has_animations && !load_layouts(capabilities->at("animations"), animation_layouts, "animation", error)) return false;
	if (has_effects && !load_layouts(capabilities->at("effects"), effect_layouts, "effect animation", error)) return false;
	elysia::io::AnimationConfigLoader animation_loader;
	elysia::io::EffectDefinitionConfigLoader effect_loader;
	for (size_t index = 0; index < entity_manifest.entities.size(); ++index)
	{
		const auto& entity = entity_manifest.entities[index];
		if ((has_animations || has_effects) && entity.animation_layout.empty())
		{
			error = "Animated entity content failed: entity animation_layout is missing: " + entity.id;
			return false;
		}
		if (has_animations)
		{
			const auto layout = animation_layouts.find(entity.animation_layout);
			if (layout == animation_layouts.end()) { error = "Animated entity content failed: unknown animation layout: " + entity.id; return false; }
			std::filesystem::path config_path;
			if (!resolve_template(animation_template, "animation_config_template", entity.asset_key, config_path, error)) return false;
			elysia::io::AnimationConfig config;
			if (!animation_loader.load(config_path, layout->second, config)) { error = "Animated entity content failed: animation config load failed."; return false; }
			content.animation_entries.push_back({content.entities[index], std::move(config)});
		}
		if (has_effects)
		{
			const auto layout = effect_layouts.find(entity.animation_layout);
			if (layout == effect_layouts.end()) { error = "Animated entity content failed: unknown effect animation layout: " + entity.id; return false; }
			std::filesystem::path animation_path, effect_path;
			if (!resolve_template(effect_animation_template, "effect_animation_config_template", entity.asset_key, animation_path, error)
				|| !resolve_template(effect_info_template, "effect_info_template", entity.asset_key, effect_path, error)) return false;
			elysia::io::AnimationConfig animation_config;
			if (!animation_loader.load(animation_path, layout->second, animation_config)) { error = "Animated entity content failed: effect animation config load failed."; return false; }
			elysia::io::EffectDefinitionConfig effect_config;
			if (!effect_loader.load(effect_path, animation_config, effect_config)) { error = "Animated entity content failed: effect info load failed."; return false; }
			content.effect_entries.push_back({content.entities[index], std::move(animation_config), std::move(effect_config)});
		}
	}
	return true;
}
}
