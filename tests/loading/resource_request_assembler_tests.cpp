#define SDL_MAIN_HANDLED

#include "engine/animation/animation_manager.h"
#include "engine/effects/effect_manager.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/config_load_pipeline.h"
#include "engine/loading/resource_load_plan.h"
#include "engine/loading/resource_request_assembler.h"
#include "engine/resources/atlas/atlas.h"
#include "tests/support/test_assertions.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace
{
using moonline::tests::require;

void test_runtime_resource_request_assembly()
{
    elysia::io::PathManager* path_manager = elysia::io::PathManager::instance();
    require(path_manager->init(), "path manager must initialize from the project root");

    elysia::loading::ConfigLoadResult config_result;
    elysia::loading::ConfigLoadPipeline config_load_pipeline;
    require(config_load_pipeline.load(path_manager->content_registry(), config_result),
        "config pipeline must load content before assembling resource requests");

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
	require(atlas_request->source_path == path_manager->textures() / "test" / "frame_group.png"
		&& atlas_request->source_type == elysia::resources::AtlasSourceType::HorizontalStrip,
		"test.animation atlas must use the configured horizontal strip");
	require(atlas_request->frame_count == 14,
		"test.animation atlas must have fourteen frames");

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

	const auto runtime_slime_attack_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "slime.attack";
		});
	require(runtime_slime_attack_request != load_plan.atlas_build_requests().end()
		&& runtime_slime_attack_request->frame_filename_prefix == "Slime_attack"
		&& runtime_slime_attack_request->source_type == elysia::resources::AtlasSourceType::FrameDirectory,
		"enabled enemies module must contribute enemy atlas requests to the runtime plan");

	const auto flying_demon_idle_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "flying_demon.idle";
		});
	require(flying_demon_idle_request != load_plan.atlas_build_requests().end()
		&& flying_demon_idle_request->source_type == elysia::resources::AtlasSourceType::HorizontalStrip
		&& flying_demon_idle_request->source_path
			== path_manager->textures() / "enemy" / "normal" / "FlyingDemon" / "idle" / "idle.png"
		&& flying_demon_idle_request->frame_count == 4,
		"FlyingDemon idle must resolve to its entity-wide horizontal strip source");

	const size_t flying_demon_request_count = static_cast<size_t>(std::count_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key.starts_with("flying_demon.")
				&& request.source_type == elysia::resources::AtlasSourceType::HorizontalStrip;
		}));
	require(flying_demon_request_count == 5,
		"FlyingDemon must contribute five horizontal strip animation requests");

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
		load_plan.animation_effect_build_requests().begin(),
		load_plan.animation_effect_build_requests().end(),
		[](const elysia::resources::AnimationEffectBuildRequest& request)
		{
			return request.effect_key == "aozaki_aoko.effect.attack_air.0";
		});
	require(aoko_no_effect_request == load_plan.animation_effect_build_requests().end(),
		"Aoko no_effects markers must suppress effect requests");

	elysia::resources::Atlas atlas("test.animation");
	elysia::animation::AnimationManager* animation_manager =
		elysia::animation::AnimationManager::instance();
	require(animation_manager->register_animation(*animation_request, &atlas),
		"test.animation request must register with the animation manager");
	require(animation_manager->create_animation("test.animation") != nullptr,
		"test.animation must create a playback instance after registration");

	const auto effect_request = std::find_if(
		load_plan.animation_effect_build_requests().begin(),
		load_plan.animation_effect_build_requests().end(),
		[](const elysia::resources::AnimationEffectBuildRequest& request)
		{
			return request.effect_key == "effect.test";
		});
	require(effect_request != load_plan.animation_effect_build_requests().end(),
		"effect.test must create an effect request");
	require(effect_request->animation_key == "test.animation",
		"effect.test must reference test.animation");

	elysia::effects::EffectManager* effect_manager = elysia::effects::EffectManager::instance();
	require(effect_manager->register_animation_effect(*effect_request),
		"effect.test request must register with the effect manager");
	elysia::effects::AnimationEffectSpawnRequest effect_spawn_request;
	effect_spawn_request.effect_key = "effect.test";
	require(effect_manager->create_animation_effect(effect_spawn_request) != nullptr,
		"effect.test must create an effect instance after registration");
}
}

int main()
{
    test_runtime_resource_request_assembly();
    std::cout << "resource request assembler tests passed\n";
    return EXIT_SUCCESS;
}
