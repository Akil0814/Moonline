#pragma once

#include "../resources/resource_types.h"

#include <cstddef>
#include <vector>

class ResourceLoadPlan
{
public:
	std::vector<AtlasLoadRequest>& atlas_requests()
	{
		return _atlas_requests;
	}

	const std::vector<AtlasLoadRequest>& atlas_requests() const
	{
		return _atlas_requests;
	}

	std::vector<AnimationBuildRequest>& animation_build_requests()
	{
		return _animation_build_requests;
	}

	const std::vector<AnimationBuildRequest>& animation_build_requests() const
	{
		return _animation_build_requests;
	}

	void clear()
	{
		_atlas_requests.clear();
		_animation_build_requests.clear();
	}

	[[nodiscard]] bool empty() const
	{
		return _atlas_requests.empty() && _animation_build_requests.empty();
	}

	[[nodiscard]] size_t total_request_count() const
	{
		return _atlas_requests.size() + _animation_build_requests.size();
	}

private:
	std::vector<AtlasLoadRequest> _atlas_requests;
	std::vector<AnimationBuildRequest> _animation_build_requests;
};
