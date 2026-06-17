#include "resource_request_assembler.h"

#include "config_load_pipeline.h"
#include "resource_load_plan.h"
#include "../resources/pipeline/resource_request_builder.h"

bool ResourceRequestAssembler::assemble(
	const ConfigLoadResult& config_result,
	ResourceLoadPlan& out_plan
) const
{
	out_plan.clear();

	ResourceRequestBuilder request_builder;
	for (const CharacterAnimationContentEntry& entry : config_result.character_animation_entries)
	{
		if (!request_builder.append_character_animation_requests(
			entry._character_config,
			entry._animation_config,
			out_plan.atlas_requests(),
			out_plan.animation_build_requests()))
		{
			out_plan.clear();
			return false;
		}
	}

	if (!request_builder.append_font_requests(
		config_result.font_manifest,
		out_plan.font_requests()))
	{
		out_plan.clear();
		return false;
	}

	if (!request_builder.append_audio_requests(
		config_result.audio_manifest,
		out_plan.sound_requests(),
		out_plan.music_requests()))
	{
		out_plan.clear();
		return false;
	}

	return true;
}
