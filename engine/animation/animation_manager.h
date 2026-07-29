#pragma once

#include "animation.h"
#include "../resources/resource_service.h"
#include "../tools/singleton.h"
#include "../resources/resource_types.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace elysia::animation
{
struct AnimationDefinition
{
	std::string animation_key;
	std::string atlas_key;
	double fps = 10.0;
	bool loop = true;
	size_t segment_index = 0;
	const elysia::resources::Atlas* atlas = nullptr;
};

class AnimationManager : public elysia::tools::Singleton<AnimationManager>
{
	friend elysia::tools::Singleton<AnimationManager>;

public:
	bool register_animation(const elysia::resources::AnimationBuildRequest& request,const elysia::resources::Atlas* atlas);
	bool register_animations(const std::vector<elysia::resources::AnimationBuildRequest>& requests,
		const elysia::resources::ResourceService& resource_service);

	const AnimationDefinition* find_definition(const std::string_view& key) const;
	std::unique_ptr<Animation> create_animation(const std::string_view& key) const;
	void clear() noexcept;

private:
	std::unordered_map<std::string, AnimationDefinition> _definitions;
};

}
