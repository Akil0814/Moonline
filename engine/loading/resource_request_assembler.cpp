#include "../tools/logger.h"
#include "resource_request_assembler.h"

#include "config_load_pipeline.h"
#include "resource_load_plan.h"
#include "../io/path/path_manager.h"
#include "../resources/pipeline/resource_request_builder.h"
namespace elysia::loading
{
bool ResourceRequestAssembler::assemble(
	const ConfigLoadResult& config_result,
	ResourceLoadPlan& out_plan
) const
{
	out_plan.clear();

	elysia::resources::ResourceRequestBuilder request_builder;
	for (const elysia::io::CharacterAnimationContentEntry& entry : config_result.character_animation_entries)
	{
		const size_t animation_atlas_count_before = out_plan.atlas_build_requests().size();
		const size_t animation_count_before = out_plan.animation_build_requests().size();

		if (!request_builder.append_character_animation_requests(
			entry.character_config,
			entry.animation_config,
			out_plan.atlas_build_requests(),
			out_plan.animation_build_requests()))
		{
			out_plan.clear();
			return false;
		}

		ELYSIA_LOG_ERROR("resource","Character animation requests built: atlases="
			<< (out_plan.atlas_build_requests().size() - animation_atlas_count_before)
			<< ", animations="
			<< (out_plan.animation_build_requests().size() - animation_count_before));

		const size_t effect_atlas_count_before = out_plan.atlas_build_requests().size();
		const size_t effect_animation_count_before = out_plan.animation_build_requests().size();
		const size_t effect_count_before = out_plan.effect_build_requests().size();

		if (!request_builder.append_character_effect_requests(
			entry.character_config,
			entry.animation_config,
			config_result.character_effect_layout,
			out_plan.atlas_build_requests(),
			out_plan.animation_build_requests(),
			out_plan.effect_build_requests()))
		{
			out_plan.clear();
			return false;
		}

		ELYSIA_LOG_ERROR("resource","Effect requests built: atlases="
			<< (out_plan.atlas_build_requests().size() - effect_atlas_count_before)
			<< ", animations="
			<< (out_plan.animation_build_requests().size() - effect_animation_count_before)
			<< ", effects="
			<< (out_plan.effect_build_requests().size() - effect_count_before));
	}

	const size_t texture_count_before = out_plan.texture_requests().size();
	const std::filesystem::path textures_root = elysia::io::PathManager::instance()->textures();
	if (!request_builder.append_texture_manifest_requests(
		config_result.ui_texture_manifest,
		"ui",
		textures_root / "ui",
		out_plan.texture_requests()))
	{
		out_plan.clear();
		return false;
	}

	if (!request_builder.append_texture_manifest_requests(
		config_result.map_texture_manifest,
		"map",
		textures_root / "map",
		out_plan.texture_requests()))
	{
		out_plan.clear();
		return false;
	}

	for (const elysia::io::CharacterAnimationContentEntry& entry : config_result.character_animation_entries)
	{
		if (!request_builder.append_character_texture_requests(
			entry.character_config,
			config_result.character_texture_layout,
			out_plan.texture_requests()))
		{
			out_plan.clear();
			return false;
		}
	}

	ELYSIA_LOG_ERROR("resource","Texture requests built: "
		<< (out_plan.texture_requests().size() - texture_count_before));

	if (!request_builder.append_font_requests(
		config_result.font_manifest,
		out_plan.font_requests()))
	{
		out_plan.clear();
		return false;
	}

	ELYSIA_LOG_ERROR("resource","Font requests built: "
		<< out_plan.font_requests().size());

	if (!request_builder.append_audio_requests(
		config_result.audio_manifest,
		out_plan.sound_requests(),
		out_plan.music_requests()))
	{
		out_plan.clear();
		return false;
	}

	for (const elysia::io::CharacterAnimationContentEntry& entry : config_result.character_animation_entries)
	{
		if (!request_builder.append_character_audio_requests(
			entry.character_config,
			config_result.character_audio_layout,
			out_plan.sound_requests()))
		{
			out_plan.clear();
			return false;
		}
	}

	ELYSIA_LOG_ERROR("resource","Audio requests built: sounds=" << out_plan.sound_requests().size()
		<< ", music=" << out_plan.music_requests().size());

	return true;
}


}
