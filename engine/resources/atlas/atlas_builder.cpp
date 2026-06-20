#include "atlas_builder.h"

#include <iostream>

namespace elysia::resources
{
bool AtlasBuilder::build_atlas(
	const AtlasBuildRequest& request,
	const std::vector<AtlasCommittedFrame>& committed_frames,
	Atlas& atlas
) const
{
	if (request.atlas_key.empty())
	{
		std::cout << "Build atlas failed: atlas key is empty." << std::endl;
		return false;
	}

	if (request.frame_count == 0)
	{
		std::cout << "Build atlas failed: frame count is zero: "
			<< request.atlas_key << std::endl;
		return false;
	}

	if (committed_frames.size() != request.frame_count)
	{
		std::cout << "Build atlas failed: texture count mismatch: "
			<< request.atlas_key << ", expected " << request.frame_count
			<< ", actual " << committed_frames.size() << std::endl;
		return false;
	}

	atlas.clear();
	atlas.set_name(request.atlas_key);

	for (size_t index = 0; index < committed_frames.size(); ++index)
	{
		const AtlasCommittedFrame& committed_frame = committed_frames[index];
		if (!committed_frame.texture)
		{
			std::cout << "Build atlas failed: texture is invalid: "
				<< request.atlas_key << ", frame " << index << std::endl;
			return false;
		}

		if (committed_frame.frame_index != index)
		{
			std::cout << "Build atlas failed: frame index mismatch: "
				<< request.atlas_key << ", expected " << index
				<< ", actual " << committed_frame.frame_index << std::endl;
			return false;
		}

		if (!atlas.add_frame(committed_frame.frame_path, committed_frame.texture))
		{
			std::cout << "Build atlas failed: add frame failed: "
				<< committed_frame.frame_path << std::endl;
			return false;
		}
	}

	return true;
}

}
