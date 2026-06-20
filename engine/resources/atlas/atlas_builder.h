#pragma once

#include "atlas.h"
#include "../resource_types.h"

#include <filesystem>
#include <vector>

namespace elysia::resources
{
struct AtlasCommittedFrame
{
	std::filesystem::path frame_path;
	SDL_Texture* texture = nullptr;
	size_t frame_index = 0;
};

class AtlasBuilder
{
public:
	bool build_atlas(
		const AtlasBuildRequest& request,
		const std::vector<AtlasCommittedFrame>& committed_frames,
		Atlas& atlas
	) const;
};

}
