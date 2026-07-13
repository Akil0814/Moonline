#define SDL_MAIN_HANDLED

#include "../engine/animation/animation_manager.h"
#include "../engine/effects/effect_manager.h"
#include "../engine/io/loaders/content_registry_loader.h"
#include "../engine/io/json/json_loader.h"
#include "../engine/io/path/path_manager.h"
#include "../engine/loading/config_load_pipeline.h"
#include "../engine/loading/resource_load_plan.h"
#include "../engine/loading/resource_request_assembler.h"
#include "../engine/resources/atlas/atlas.h"
#include "../engine/resources/atlas/atlas_build_preparer.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
void require(bool condition, const char* message)
{
	if (condition)
		return;

	std::cerr << "FAILED: " << message << '\n';
	std::exit(EXIT_FAILURE);
}
}

int main()
{
	const std::filesystem::path atlas_test_root =
		std::filesystem::temp_directory_path() / "moonline_atlas_build_preparer_tests";
	std::filesystem::remove_all(atlas_test_root);
	std::filesystem::create_directories(atlas_test_root / "named");
	std::filesystem::create_directories(atlas_test_root / "legacy");
	std::ofstream(atlas_test_root / "named" / "RyougiShiki_idle_000.png").put('\0');
	std::ofstream(atlas_test_root / "named" / "RyougiShiki_idle_001.png").put('\0');
	std::ofstream(atlas_test_root / "legacy" / "frame_002.png").put('\0');
	std::ofstream(atlas_test_root / "legacy" / "frame_001.png").put('\0');

	elysia::resources::AtlasBuildPreparer atlas_build_preparer;
	std::vector<elysia::resources::AtlasFramePrepareTask> atlas_tasks;
	elysia::resources::AtlasBuildRequest named_request;
	named_request.atlas_key = "ryougi_shiki.idle";
	named_request.directory_path = atlas_test_root / "named";
	named_request.frame_count = 2;
	named_request.frame_filename_prefix = "RyougiShiki_idle";
	require(atlas_build_preparer.expand_build_request(named_request, atlas_tasks),
		"named atlas frames must be inferred from the request prefix");
	require(atlas_tasks.size() == 2
		&& atlas_tasks[0].frame_path.filename() == "RyougiShiki_idle_000.png"
		&& atlas_tasks[1].frame_path.filename() == "RyougiShiki_idle_001.png",
		"named atlas frames must retain generated numeric order");
	named_request.frame_count = 3;
	require(!atlas_build_preparer.expand_build_request(named_request, atlas_tasks),
		"named atlas loading must fail when an inferred frame is missing");

	elysia::resources::AtlasBuildRequest legacy_request;
	legacy_request.atlas_key = "legacy";
	legacy_request.directory_path = atlas_test_root / "legacy";
	legacy_request.frame_count = 2;
	require(atlas_build_preparer.expand_build_request(legacy_request, atlas_tasks),
		"legacy atlas loading must retain directory-scan compatibility");
	require(atlas_tasks[0].frame_path.filename() == "frame_001.png"
		&& atlas_tasks[1].frame_path.filename() == "frame_002.png",
		"legacy atlas frames must remain filename-sorted");
	std::filesystem::remove_all(atlas_test_root);

	elysia::io::PathManager* path_manager = elysia::io::PathManager::instance();
	require(path_manager->init(), "path manager must initialize from the project root");
	const std::filesystem::path registry_test_root =
		std::filesystem::temp_directory_path() / "moonline_content_registry_tests";
	std::filesystem::remove_all(registry_test_root);
	std::filesystem::create_directories(registry_test_root);
	const auto write_registry = [&registry_test_root](const char* file_name, const std::string& manifests)
	{
		const std::filesystem::path path = registry_test_root / file_name;
		std::ofstream file(path);
		file << "{\"manifests\":" << manifests << "}";
		return path;
	};
	const std::string required_manifests = R"({"required":{"fonts":"configs/manifests/fonts_manifest.json","audio":"configs/manifests/audio_manifest.json","i18n":"configs/manifests/i18n_manifest.json","textures":"configs/manifests/textures_manifest.json","animations":"configs/manifests/animations_manifest.json","effects":"configs/manifests/effects_manifest.json","config_manifest":"configs/manifests/config_manifest.json"}})";

	elysia::io::ContentRegistryLoader content_registry_loader;
	elysia::io::ContentRegistry content_registry;
	elysia::io::JsonLoader config_manifest_loader;
	require(static_cast<bool>(config_manifest_loader.open_file(
		path_manager->to_config_path("manifests/config_manifest.json"))),
		"config manifest must be valid JSON");
	require(config_manifest_loader.root().contains("configs"),
		"config manifest must declare runtime config paths");
	elysia::io::JsonLoader input_config_loader;
	require(static_cast<bool>(input_config_loader.open_file(
		path_manager->to_config_path("global/input_config.json"))),
		"input config placeholder must be valid JSON");
	elysia::io::JsonLoader game_config_loader;
	require(static_cast<bool>(game_config_loader.open_file(
		path_manager->to_config_path("global/game_config.json"))),
		"game config placeholder must be valid JSON");
	require(content_registry_loader.load(
		write_registry("without_characters.json", required_manifests), content_registry),
		"content registry must allow no additional modules");
	require(content_registry.additional_modules.empty(),
		"content registry without additional modules must remain empty");
	require(!content_registry_loader.load(
		write_registry("missing_required.json", R"({"additional":{}})"), content_registry),
		"content registry must reject a missing required section");
	require(!content_registry_loader.load(
		write_registry("unknown_required.json", R"({"required":{"unknown":"x"}})"), content_registry),
		"content registry must reject unknown required keys");
	require(!content_registry_loader.load(
		write_registry("legacy_config_documents.json", R"({"required":{"fonts":"configs/manifests/fonts_manifest.json","audio":"configs/manifests/audio_manifest.json","i18n":"configs/manifests/i18n_manifest.json","textures":"configs/manifests/textures_manifest.json","animations":"configs/manifests/animations_manifest.json","effects":"configs/manifests/effects_manifest.json","config_documents":"configs/manifests/config_documents.json"}})"), content_registry),
		"content registry must reject the legacy config_documents key");

	elysia::loading::ConfigLoadResult config_result;
	elysia::loading::ConfigLoadPipeline config_load_pipeline;
	require(config_load_pipeline.load(path_manager->content_registry(), config_result),
		"config pipeline must load the generic animation manifest");
	require(config_result.characters.has_value(),
		"configured characters module must be loaded");

	elysia::loading::ConfigLoadResult core_only_config_result;
	require(config_load_pipeline.load(
		write_registry("core_only.json", required_manifests), core_only_config_result),
		"config pipeline must load without the characters module");
	require(!core_only_config_result.characters.has_value(),
		"omitted characters module must not produce character content");
	elysia::loading::ResourceLoadPlan core_only_load_plan;
	elysia::loading::ResourceRequestAssembler core_only_request_assembler;
	require(core_only_request_assembler.assemble(core_only_config_result, core_only_load_plan),
		"request assembler must support core-only content");
	const auto core_only_character_request = std::find_if(
		core_only_load_plan.atlas_build_requests().begin(),
		core_only_load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key.starts_with("ryougi_shiki.");
		});
	require(core_only_character_request == core_only_load_plan.atlas_build_requests().end(),
		"core-only content must not request character atlases");

	elysia::loading::ConfigLoadResult invalid_module_config_result;
	require(!config_load_pipeline.load(
		write_registry("unknown_module.json", R"({"required":{"fonts":"configs/manifests/fonts_manifest.json","audio":"configs/manifests/audio_manifest.json","i18n":"configs/manifests/i18n_manifest.json","textures":"configs/manifests/textures_manifest.json","animations":"configs/manifests/animations_manifest.json","effects":"configs/manifests/effects_manifest.json","config_manifest":"configs/manifests/config_manifest.json"},"additional":{"unknown":{}}})"), invalid_module_config_result),
		"config pipeline must reject unknown additional modules");
	require(!config_load_pipeline.load(
		write_registry("incomplete_characters.json", R"({"required":{"fonts":"configs/manifests/fonts_manifest.json","audio":"configs/manifests/audio_manifest.json","i18n":"configs/manifests/i18n_manifest.json","textures":"configs/manifests/textures_manifest.json","animations":"configs/manifests/animations_manifest.json","effects":"configs/manifests/effects_manifest.json","config_manifest":"configs/manifests/config_manifest.json"},"additional":{"characters":{}}})"), invalid_module_config_result),
		"config pipeline must reject incomplete characters module config");
	std::filesystem::remove_all(registry_test_root);

	elysia::loading::ResourceLoadPlan load_plan;
	elysia::loading::ResourceRequestAssembler request_assembler;
	require(request_assembler.assemble(config_result, load_plan),
		"request assembler must build generic animation requests");

	const auto atlas_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "test.animation";
		});
	require(atlas_request != load_plan.atlas_build_requests().end(),
		"test.animation must create an atlas request");
	require(atlas_request->directory_path == path_manager->textures() / "test",
		"test.animation atlas must use assets/textures/test");
	require(atlas_request->frame_count == 13,
		"test.animation atlas must have thirteen frames");

	const auto animation_request = std::find_if(
		load_plan.animation_build_requests().begin(),
		load_plan.animation_build_requests().end(),
		[](const elysia::resources::AnimationBuildRequest& request)
		{
			return request.animation_key == "test.animation";
		});
	require(animation_request != load_plan.animation_build_requests().end(),
		"test.animation must create an animation request");
	require(animation_request->atlas_key == "test.animation"
		&& animation_request->fps == 10.0 && animation_request->loop,
		"test.animation must preserve its atlas key and playback settings");

	const auto ryougi_start_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "ryougi_shiki.start";
		});
	require(ryougi_start_request != load_plan.atlas_build_requests().end()
		&& ryougi_start_request->frame_filename_prefix == "RyougiShiki_start",
		"RyougiShiki start must use its inferred frame prefix");

	const auto ryougi_getup_air_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "ryougi_shiki.getup_air";
		});
	require(ryougi_getup_air_request != load_plan.atlas_build_requests().end(),
		"RyougiShiki getup_air must create an atlas request");

	const auto ryougi_special_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key.find("ryougi_shiki.attack_special") != std::string::npos;
		});
	require(ryougi_special_request == load_plan.atlas_build_requests().end(),
		"RyougiShiki special attacks must not create runtime atlas requests");

	const auto aoko_no_effect_request = std::find_if(
		load_plan.effect_build_requests().begin(),
		load_plan.effect_build_requests().end(),
		[](const elysia::resources::EffectBuildRequest& request)
		{
			return request.effect_key == "aozaki_aoko.effect.attack_air.0";
		});
	require(aoko_no_effect_request == load_plan.effect_build_requests().end(),
		"Aoko no_effects markers must suppress effect requests");

	elysia::resources::Atlas atlas("test.animation");
	elysia::animation::AnimationManager* animation_manager =
		elysia::animation::AnimationManager::instance();
	require(animation_manager->register_animation(*animation_request, &atlas),
		"test.animation request must register with the animation manager");
	require(animation_manager->create_animation("test.animation") != nullptr,
		"test.animation must create a playback instance after registration");

	const auto effect_request = std::find_if(
		load_plan.effect_build_requests().begin(),
		load_plan.effect_build_requests().end(),
		[](const elysia::resources::EffectBuildRequest& request)
		{
			return request.effect_key == "effect.test";
		});
	require(effect_request != load_plan.effect_build_requests().end(),
		"effect.test must create an effect request");
	require(effect_request->animation_key == "test.animation",
		"effect.test must reference test.animation");

	elysia::effects::EffectManager* effect_manager = elysia::effects::EffectManager::instance();
	require(effect_manager->register_effect(*effect_request),
		"effect.test request must register with the effect manager");
	elysia::effects::EffectSpawnRequest effect_spawn_request;
	effect_spawn_request.effect_key = "effect.test";
	require(effect_manager->create_effect(effect_spawn_request) != nullptr,
		"effect.test must create an effect instance after registration");

	return EXIT_SUCCESS;
}
