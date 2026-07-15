#define SDL_MAIN_HANDLED

#include "engine/io/loaders/content_registry_loader.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/animated_entity_content_loader.h"
#include "engine/loading/content_manifest_pipeline.h"
#include "tests/support/test_assertions.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
using moonline::tests::require;

std::string json_path(const std::filesystem::path& path)
{
	return path.generic_string();
}

std::filesystem::path write_file(
	const std::filesystem::path& root,
	const std::string& name,
	const std::string& contents)
{
	const auto path = root / name;
	std::ofstream(path) << contents;
	return path;
}

std::string required_manifests()
{
	return R"("required":{"configs":"configs/manifests/config_manifest.json","fonts":"configs/manifests/fonts_manifest.json","audio":"configs/manifests/audio_manifest.json","i18n":"configs/manifests/i18n_manifest.json","textures":"configs/manifests/textures_manifest.json","animations":"configs/manifests/animations_manifest.json","effects":"configs/manifests/effects_manifest.json"})";
}

std::filesystem::path write_registry(
	const std::filesystem::path& root,
	const std::string& name,
	const std::string& additional = {})
{
	std::string manifests = "{" + required_manifests();
	if (!additional.empty()) manifests += ",\"additional\":" + additional;
	manifests += "}";
	return write_file(root, name,
		R"({"bootstrap":{"app_config":"configs/global/app_config.json","preload_manifest":"configs/manifests/preload_manifest.json"},"manifests":)"
		+ manifests + "}");
}

void test_current_modules_load_through_generic_pipeline()
{
	auto* path_manager = elysia::io::PathManager::instance();
	require(path_manager->init(), "path manager must initialize from the project root");

	elysia::loading::ContentManifestPipeline pipeline;
	elysia::loading::ContentManifestResult result;
	elysia::io::ContentRegistry registry;
	require(elysia::io::ContentRegistryLoader{}.load(path_manager->content_registry(), registry),
		"current content registry must parse before it is supplied to the content pipeline");
	require(pipeline.load(registry, result),
		"current content registry must load through the generic additional-module pipeline");
	require(result.config_snapshot != nullptr,
		"content manifest loading must build the deferred generic config snapshot");
	require(result.additional_modules.size() == 3
		&& result.additional_modules.contains("characters")
		&& result.additional_modules.contains("character_effects")
		&& result.additional_modules.contains("enemies"),
		"additional modules must be stored deterministically by arbitrary manifest name");

	const auto& characters = result.additional_modules.at("characters");
	require(characters.name == "characters"
		&& characters.key_namespace.empty()
		&& characters.animation_entries.size() == 3
		&& characters.effect_entries.empty()
		&& characters.texture_entries.size() == 3
		&& characters.audio_entries.size() == 3,
		"characters module must independently expose its optional animation, texture and audio capabilities");

	const auto& effects = result.additional_modules.at("character_effects");
	require(effects.name == "character_effects"
		&& effects.key_namespace == "effect"
		&& effects.animation_entries.size() == 3
		&& effects.effect_entries.size() == 3
		&& effects.texture_entries.empty()
		&& effects.audio_entries.empty(),
		"character effects must be a regular Animation+Effect additional module");

	const auto& enemies = result.additional_modules.at("enemies");
	require(enemies.animation_entries.size() == 5
		&& enemies.effect_entries.empty()
		&& enemies.texture_entries.empty()
		&& enemies.audio_entries.empty(),
		"an animation-only additional module must load without unrelated capabilities");
}

void test_arbitrary_and_empty_additional_module()
{
	const auto root = std::filesystem::temp_directory_path() / "moonline_generic_module_tests";
	std::filesystem::remove_all(root);
	std::filesystem::create_directories(root);
	const auto entities = write_file(root, "npcs.json",
		R"({"entities":[{"id":"npc_1","asset_key":"Npc_1"}]})");
	const auto module = write_file(root, "npc_module.json",
		"{\"entities\":\"" + json_path(entities)
		+ "\",\"key_namespace\":\"\",\"capabilities\":{}}");

	elysia::loading::AnimatedEntityContentLoader loader;
	elysia::io::EntityContentModule content;
	std::string error;
	require(loader.load("npcs", module, content, error)
		&& content.name == "npcs"
		&& content.entities.size() == 1
		&& content.entities.front().id == "npc_1"
		&& content.animation_entries.empty()
		&& content.effect_entries.empty()
		&& content.texture_entries.empty()
		&& content.audio_entries.empty(),
		"an arbitrary module name with an empty capability object must be valid");
	require(loader.load("", module, content, error)
		&& content.entities.front().origin.scope == elysia::resources::ResourceOriginScope::AdditionalModule
		&& content.entities.front().origin.module.empty(),
		"an empty additional module name must remain distinguishable from core origin scope");
	require(loader.load("core", module, content, error)
		&& content.entities.front().origin.scope == elysia::resources::ResourceOriginScope::AdditionalModule
		&& content.entities.front().origin.module == "core",
		"an additional module literally named core must remain distinguishable from core origin scope");

	elysia::loading::ContentManifestPipeline pipeline;
	elysia::loading::ContentManifestResult result;
	const auto registry = write_registry(root, "registry.json",
		"{\"npcs\":\"" + json_path(module) + "\"}");
	elysia::io::ContentRegistry content_registry;
	require(elysia::io::ContentRegistryLoader{}.load(registry, content_registry),
		"arbitrary-module registry must parse before it is supplied to the content pipeline");
	require(pipeline.load(content_registry, result)
		&& result.additional_modules.size() == 1
		&& result.additional_modules.contains("npcs")
		&& result.additional_modules.at("npcs").entities.size() == 1,
		"ContentManifestPipeline must dispatch arbitrary additional names through the same loader");

	std::filesystem::remove_all(root);
}

void test_module_schema_rejections()
{
	const auto root = std::filesystem::temp_directory_path() / "moonline_module_schema_tests";
	std::filesystem::remove_all(root);
	std::filesystem::create_directories(root);
	const auto entities = write_file(root, "entities.json",
		R"({"entities":[{"id":"npc","asset_key":"Npc"}]})");
	const std::string prefix = "{\"entities\":\"" + json_path(entities) + "\",\"key_namespace\":\"\",";

	elysia::loading::AnimatedEntityContentLoader loader;
	elysia::io::EntityContentModule content;
	std::string error;
	require(!loader.load("effects_only", write_file(root, "effects_only.json",
		prefix + R"("capabilities":{"effects":{"config_template":"configs/{asset_key}/effects.json"}}})"), content, error)
		&& error.find("effects requires animations") != std::string::npos,
		"effects capability must require animations in the same module");
	require(!loader.load("unknown", write_file(root, "unknown_capability.json",
		prefix + R"("capabilities":{"particles":{}}})"), content, error)
		&& error.find("unknown capability") != std::string::npos,
		"module manifests must reject capabilities outside animations/effects/textures/audio");
	require(!loader.load("legacy", write_file(root, "legacy_resources.json",
		prefix + R"("resources":{},"capabilities":{}})"), content, error),
		"module manifests must reject the removed shared resources object");
	require(!loader.load("missing_namespace", write_file(root, "missing_namespace.json",
		"{\"entities\":\"" + json_path(entities) + "\",\"capabilities\":{}}"), content, error),
		"module manifests must require key_namespace even when it is empty");
	require(!loader.load("unknown_field", write_file(root, "unknown_animation_field.json",
		prefix + R"("capabilities":{"animations":{"texture_root":"textures/{asset_key}","config_template":"configs/{asset_key}.json","layouts":{},"legacy":true}}})"), content, error)
		&& error.find("unknown animations capability field") != std::string::npos,
		"capability objects must reject unknown fields");

	std::filesystem::remove_all(root);
}

void test_texture_only_and_audio_only_modules()
{
	const auto root = std::filesystem::temp_directory_path() / "moonline_single_capability_module_tests";
	std::filesystem::remove_all(root);
	std::filesystem::create_directories(root);
	elysia::loading::AnimatedEntityContentLoader loader;
	elysia::io::EntityContentModule content;
	std::string error;

	const auto texture_module = write_file(root, "textures.json",
		R"({"entities":"configs/character/characters_manifest.json","key_namespace":"portrait","capabilities":{"textures":{"texture_root":"textures/character/{asset_key}","layout":"configs/character/layouts/character_texture_layout.json"}}})");
	require(loader.load("portraits", texture_module, content, error)
		&& content.animation_entries.empty() && content.effect_entries.empty()
		&& content.texture_entries.size() == 3 && content.audio_entries.empty(),
		"a texture-only module must load without Animation, Effect, or Audio capabilities");

	const auto audio_module = write_file(root, "audio.json",
		R"({"entities":"configs/character/characters_manifest.json","key_namespace":"voice","capabilities":{"audio":{"audio_root":"audio/character/{asset_key}","layout":"configs/character/layouts/character_audio_layout.json"}}})");
	require(loader.load("voices", audio_module, content, error)
		&& content.animation_entries.empty() && content.effect_entries.empty()
		&& content.texture_entries.empty() && content.audio_entries.size() == 3,
		"an audio-only module must load without Animation, Effect, or Texture capabilities");

	std::filesystem::remove_all(root);
}

void test_animation_frame_prefix_template_rules()
{
	const auto root = std::filesystem::temp_directory_path()
		/ "moonline_animation_prefix_module_tests";
	std::filesystem::remove_all(root);
	std::filesystem::create_directories(root);
	elysia::loading::AnimatedEntityContentLoader loader;
	elysia::io::EntityContentModule content;
	std::string error;

	const std::string enemy_animation_fields =
		R"("texture_root":"textures/enemy/normal/{asset_key}","config_template":"configs/enemy/normal/{asset_key}_animation_info.json","layouts":{"normal":"configs/enemy/enemy_general_animation_layout.json"})";
	require(!loader.load("missing_prefix", write_file(root, "missing_prefix.json",
		R"({"entities":"configs/enemy/enemy_manifest.json","key_namespace":"","capabilities":{"animations":{)"
		+ enemy_animation_fields + "}}}"), content, error),
		"a module containing frame-directory configs must require frame_prefix_template");
	require(!loader.load("invalid_token", write_file(root, "invalid_token.json",
		R"({"entities":"configs/enemy/enemy_manifest.json","key_namespace":"","capabilities":{"animations":{)"
		+ enemy_animation_fields + R"(,"frame_prefix_template":"{asset_key}_{animation}_{unknown}"}}})"),
		content, error),
		"frame_prefix_template must reject tokens outside the documented three-token set");

	const std::string character_animation_fields =
		R"("texture_root":"textures/character/{asset_key}","config_template":"configs/character/{asset_key}/animation_info.json","layouts":{"fighter":"configs/character/layouts/character_animation_layout.json"})";
	require(!loader.load("missing_segment_suffix", write_file(root, "missing_segment_suffix.json",
		R"({"entities":"configs/character/characters_manifest.json","key_namespace":"","capabilities":{"animations":{)"
		+ character_animation_fields + R"(,"frame_prefix_template":"{asset_key}_{animation}"}}})"),
		content, error),
		"a module with segmented animations must include segment_suffix in its frame prefix template");

	const auto strip_entities = write_file(root, "strip_entities.json",
		R"({"entities":[{"id":"flying_demon","asset_key":"FlyingDemon","animation_layout":"normal"}]})");
	require(loader.load("strip_only", write_file(root, "strip_only.json",
		"{\"entities\":\"" + json_path(strip_entities)
		+ R"(","key_namespace":"","capabilities":{"animations":{"texture_root":"textures/enemy/normal/{asset_key}","config_template":"configs/enemy/normal/{asset_key}_animation_info.json","layouts":{"normal":"configs/enemy/enemy_general_animation_layout.json"}}}})"),
		content, error)
		&& content.animation_entries.size() == 1
		&& content.animation_entries.front().frame_prefix_template.empty()
		&& content.animation_entries.front().animation_config.source_type
			== elysia::io::AnimationSourceType::HorizontalStrip,
		"a horizontal-strip-only module must not require an unused frame prefix template");

	std::filesystem::remove_all(root);
}

void test_content_registry_still_allows_core_only()
{
	const auto root = std::filesystem::temp_directory_path() / "moonline_core_only_registry_tests";
	std::filesystem::remove_all(root);
	std::filesystem::create_directories(root);
	elysia::loading::ContentManifestPipeline pipeline;
	elysia::loading::ContentManifestResult result;
	const auto registry_path = write_registry(root, "core_only.json");
	elysia::io::ContentRegistry registry;
	require(elysia::io::ContentRegistryLoader{}.load(registry_path, registry),
		"core-only registry must parse before it is supplied to the content pipeline");
	std::filesystem::remove(registry_path);
	require(pipeline.load(registry, result)
		&& result.additional_modules.empty(),
		"content pipeline must use the supplied registry snapshot without rereading its source file");
	std::filesystem::remove_all(root);
}

void test_config_manifest_registry_contract()
{
	auto* paths = elysia::io::PathManager::instance();
	require(paths->init(),"PathManager must initialize for registry contract tests");
	elysia::io::ContentRegistryLoader loader;
	elysia::io::ContentRegistry registry;
	require(loader.load(paths->content_registry(),registry)
		&& registry.required.configs == paths->assets()/"configs/manifests/config_manifest.json",
		"manifests.required.configs must be a required resolved entry");

	const auto root = std::filesystem::temp_directory_path()/"moonline_config_registry_contract_tests";
	std::filesystem::remove_all(root); std::filesystem::create_directories(root);
	const std::string missing_configs = R"({"bootstrap":{"app_config":"configs/global/app_config.json","preload_manifest":"configs/manifests/preload_manifest.json"},"manifests":{"required":{"fonts":"configs/manifests/fonts_manifest.json","audio":"configs/manifests/audio_manifest.json","i18n":"configs/manifests/i18n_manifest.json","textures":"configs/manifests/textures_manifest.json","animations":"configs/manifests/animations_manifest.json","effects":"configs/manifests/effects_manifest.json"}}})";
	require(!loader.load(write_file(root,"missing_configs.json",missing_configs),registry),
		"content registry must reject a missing manifests.required.configs");
	const std::string legacy_bootstrap = R"({"bootstrap":{"app_config":"configs/global/app_config.json","preload_manifest":"configs/manifests/preload_manifest.json","game_config_manifest":"configs/manifests/config_manifest.json"},"manifests":{)"
		+ required_manifests()+"}}";
	require(!loader.load(write_file(root,"legacy_bootstrap.json",legacy_bootstrap),registry),
		"content registry must reject the removed bootstrap.game_config_manifest field");
	std::filesystem::remove_all(root);
}
}

int main()
{
	test_current_modules_load_through_generic_pipeline();
	test_arbitrary_and_empty_additional_module();
	test_module_schema_rejections();
	test_texture_only_and_audio_only_modules();
	test_animation_frame_prefix_template_rules();
	test_content_registry_still_allows_core_only();
	test_config_manifest_registry_contract();
	std::cout << "content load pipeline tests passed\n";
	return EXIT_SUCCESS;
}
