#include "resource_request_assembler.h"

#include "resource_load_plan.h"
#include "../config/config_manager.h"
#include "../resources/pipeline/resource_request_builder.h"

bool ResourceRequestAssembler::assemble(
	const ConfigManager& config_manager,
	ResourceLoadPlan& out_plan
) const
{
	out_plan.clear();

	ResourceRequestBuilder request_builder;
	for (const CharacterAnimationContentEntry& entry : config_manager.character_animation_entries())
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

	return true;
}
