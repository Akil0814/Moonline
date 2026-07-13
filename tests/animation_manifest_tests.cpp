#define SDL_MAIN_HANDLED

#include "../engine/animation/animation_manager.h"
#include "../engine/effects/effect_manager.h"
#include "../engine/io/path/path_manager.h"
#include "../engine/loading/config_load_pipeline.h"
#include "../engine/loading/resource_load_plan.h"
#include "../engine/loading/resource_request_assembler.h"
#include "../engine/resources/atlas/atlas.h"

#include <algorithm>
#include <cstdlib>
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
	elysia::io::PathManager* path_manager = elysia::io::PathManager::instance();
	require(path_manager->init(), "path manager must initialize from the project root");

	elysia::loading::ConfigLoadResult config_result;
	elysia::loading::ConfigLoadPipeline config_load_pipeline;
	require(config_load_pipeline.load(path_manager->assets_structure(), config_result),
		"config pipeline must load the generic animation manifest");

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
