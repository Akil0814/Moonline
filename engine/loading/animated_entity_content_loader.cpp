#include "animated_entity_content_loader.h"

#include "../io/loaders/animation_config_loader.h"
#include "../io/loaders/animation_effect_layout_loader.h"
#include "../io/loaders/animation_layout_loader.h"
#include "../io/loaders/entity_audio_layout_loader.h"
#include "../io/loaders/entity_manifest_loader.h"
#include "../io/loaders/entity_texture_layout_loader.h"
#include "../io/json/json_loader.h"
#include "../io/path/path_manager.h"

#include <unordered_map>

namespace elysia::loading
{
namespace
{
bool read_path(const elysia::io::json& node, std::string_view key, std::filesystem::path& out, std::string& error)
{
	if (!node.contains(std::string(key)) || !node.at(std::string(key)).is_string()) { error = "Animated entity content failed: missing or invalid " + std::string(key) + "."; return false; }
	out = elysia::io::PathManager::instance()->to_asset_path(node.at(std::string(key)).get<std::string>());
	return true;
}

bool resolve_template(const std::filesystem::path& pattern, const std::string& asset_key, std::filesystem::path& out, std::string& error)
{
	std::string value = pattern.string();
	const size_t marker = value.find("{asset_key}");
	if (marker == std::string::npos) { error = "Animated entity content failed: animation_config_template must contain {asset_key}."; return false; }
	value.replace(marker, std::string("{asset_key}").size(), asset_key);
	out = elysia::io::PathManager::instance()->to_config_path(value);
	if (!std::filesystem::is_regular_file(out)) { error = "Animated entity content failed: animation config does not exist: " + out.string(); return false; }
	return true;
}

bool resolve_entity_texture_root(
	const std::string& pattern,
	const std::string& asset_key,
	std::filesystem::path& out,
	std::string& error
)
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
}

bool AnimatedEntityContentLoader::load(const std::filesystem::path& path, elysia::io::AnimatedEntityContent& content, std::string& error) const
{
	content = {};
	elysia::io::JsonLoader loader;
	if (!loader.open_file(path) || !loader.root().is_object()) { error = "Animated entity content failed: module manifest is invalid."; return false; }
	const elysia::io::json& root = loader.root();
	for (auto it=root.begin(); it!=root.end(); ++it) if (it.key()!="entities" && it.key()!="resources" && it.key()!="capabilities") { error="Animated entity content failed: unknown manifest key: "+it.key(); return false; }
	std::filesystem::path entities_path;
	if (!read_path(root, "entities", entities_path, error) || !std::filesystem::is_regular_file(entities_path)) { if (error.empty()) error="Animated entity content failed: entities manifest does not exist."; return false; }
	elysia::io::EntityManifest entity_manifest;
	elysia::io::EntityManifestLoader entity_loader;
	if (!entity_loader.load(entities_path, entity_manifest)) { error="Animated entity content failed: entity manifest load failed."; return false; }

	const elysia::io::json* resources = nullptr;
	if (root.contains("resources"))
	{
		if (!root.at("resources").is_object()) { error="Animated entity content failed: resources is not an object."; return false; }
		resources=&root.at("resources");
		for (auto it=resources->begin(); it!=resources->end(); ++it)
			if (it.key()!="texture_root" && it.key()!="animation_config_template" && it.key()!="audio_root")
			{ error="Animated entity content failed: unknown resource field: "+it.key(); return false; }
	}
	const elysia::io::json* capabilities = nullptr;
	if (root.contains("capabilities")) { if (!root.at("capabilities").is_object()) { error="Animated entity content failed: capabilities is not an object."; return false; } capabilities=&root.at("capabilities"); }
	if (!capabilities)
	{
		for (const auto& entity : entity_manifest.entities)
			content.entities.push_back({entity.id, entity.asset_key, {}, {}});
		return true;
	}
	for (auto it=capabilities->begin();it!=capabilities->end();++it) if (it.key()!="animations"&&it.key()!="textures"&&it.key()!="audio"&&it.key()!="effects") { error="Animated entity content failed: unknown capability: "+it.key(); return false; }
	for (auto it=capabilities->begin(); it!=capabilities->end(); ++it)
	{
		if (!it.value().is_object()) { error="Animated entity content failed: capability is not an object: "+it.key(); return false; }
		const std::string expected_key = it.key()=="animations" ? "layouts" : "layout";
		for (auto field=it.value().begin(); field!=it.value().end(); ++field)
			if (field.key()!=expected_key) { error="Animated entity content failed: unknown capability field: "+field.key(); return false; }
	}

	std::string texture_root_template;
	std::filesystem::path animation_template, audio_root;
	if (resources)
	{
		if (resources->contains("texture_root") && !resources->at("texture_root").is_string()) { error="Animated entity content failed: invalid texture_root."; return false; }
		if (resources->contains("texture_root")) texture_root_template=resources->at("texture_root").get<std::string>();
		if (resources->contains("animation_config_template") && !resources->at("animation_config_template").is_string()) { error="Animated entity content failed: invalid animation_config_template."; return false; }
		if (resources->contains("animation_config_template")) animation_template=resources->at("animation_config_template").get<std::string>();
		if (resources->contains("audio_root") && !read_path(*resources,"audio_root",audio_root,error)) return false;
	}
	const bool has_animations = capabilities->contains("animations");
	if (capabilities->contains("effects") && !has_animations) { error="Animated entity content failed: effects requires animations."; return false; }
	if ((has_animations || capabilities->contains("textures")) && texture_root_template.empty()) { error="Animated entity content failed: texture_root is required."; return false; }
	if (capabilities->contains("audio") && (audio_root.empty() || !std::filesystem::is_directory(audio_root))) { error="Animated entity content failed: audio_root is required and must exist."; return false; }
	for (const auto& entity : entity_manifest.entities)
	{
		std::filesystem::path entity_texture_root;
		if ((has_animations || capabilities->contains("textures"))
			&& !resolve_entity_texture_root(texture_root_template, entity.asset_key, entity_texture_root, error))
			return false;
		content.entities.push_back({entity.id, entity.asset_key, std::move(entity_texture_root), audio_root});
	}

	if (capabilities->contains("textures"))
	{
		const auto& node=capabilities->at("textures"); std::filesystem::path layout_path;
		if (!node.is_object() || !read_path(node,"layout",layout_path,error) || !std::filesystem::is_regular_file(layout_path)) return false;
		elysia::io::EntityTextureLayout layout; elysia::io::EntityTextureLayoutLoader layout_loader;
		if (!layout_loader.load(layout_path,layout)) { error="Animated entity content failed: texture layout load failed."; return false; }
		content.texture_layout=std::move(layout);
	}
	if (capabilities->contains("audio"))
	{
		const auto& node=capabilities->at("audio"); std::filesystem::path layout_path;
		if (!node.is_object() || !read_path(node,"layout",layout_path,error) || !std::filesystem::is_regular_file(layout_path)) return false;
		elysia::io::EntityAudioLayout layout; elysia::io::EntityAudioLayoutLoader layout_loader;
		if (!layout_loader.load(layout_path,layout)) { error="Animated entity content failed: audio layout load failed."; return false; }
		content.audio_layout=std::move(layout);
	}
	if (capabilities->contains("effects"))
	{
		const auto& node=capabilities->at("effects"); std::filesystem::path layout_path;
		if (!node.is_object() || !read_path(node,"layout",layout_path,error) || !std::filesystem::is_regular_file(layout_path)) return false;
		elysia::io::AnimationEffectLayout layout; elysia::io::AnimationEffectLayoutLoader layout_loader;
		if (!layout_loader.load(layout_path,layout)) { error="Animated entity content failed: effect layout load failed."; return false; }
		content.effect_layout=std::move(layout);
	}
	if (!has_animations) return true;
	const auto& animation_node=capabilities->at("animations");
	if (!animation_node.is_object() || !animation_node.contains("layouts") || !animation_node.at("layouts").is_object() || animation_template.empty()) { error="Animated entity content failed: animations requires layouts and animation_config_template."; return false; }
	std::unordered_map<std::string,elysia::io::AnimationLayout> layouts;
	elysia::io::AnimationLayoutLoader layout_loader;
	for(auto it=animation_node.at("layouts").begin();it!=animation_node.at("layouts").end();++it){ if(!it.value().is_string()){error="Animated entity content failed: animation layout path is invalid.";return false;} auto p=elysia::io::PathManager::instance()->to_asset_path(it.value().get<std::string>()); elysia::io::AnimationLayout layout; if(!std::filesystem::is_regular_file(p)||!layout_loader.load(p,layout)){error="Animated entity content failed: animation layout load failed.";return false;} layouts.emplace(it.key(),std::move(layout)); }
	elysia::io::AnimationConfigLoader animation_loader;
	for(size_t index=0;index<entity_manifest.entities.size();++index){ const auto& entity=entity_manifest.entities[index]; auto layout=layouts.find(entity.animation_layout); if(entity.animation_layout.empty()||layout==layouts.end()){error="Animated entity content failed: entity animation_layout is missing or unknown: "+entity.id;return false;} std::filesystem::path animation_path; if(!resolve_template(animation_template,entity.asset_key,animation_path,error))return false; elysia::io::AnimationConfig animation; if(!animation_loader.load(animation_path,layout->second,animation)){error="Animated entity content failed: animation config load failed.";return false;} content.animation_entries.push_back({content.entities[index],std::move(animation)}); }
	return true;
}
}
