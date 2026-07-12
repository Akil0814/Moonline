#include "../tools/logger.h"
#include "animation_manager.h"

#include "../resources/resource_manager.h"
namespace elysia::animation
{
bool AnimationManager::register_animation(
	const elysia::resources::AnimationBuildRequest& request,
	const elysia::resources::Atlas* atlas
)
{
	if (request.animation_key.empty())
	{
		ELYSIA_LOG_ERROR("animation","Register animation failed: animation key is empty.");
		return false;
	}

	if (request.atlas_key.empty())
	{
		ELYSIA_LOG_ERROR("animation","Register animation failed: atlas key is empty: "
			<< request.animation_key);
		return false;
	}

	if (!atlas)
	{
		ELYSIA_LOG_ERROR("animation","Register animation failed: atlas is null: "
			<< request.animation_key);
		return false;
	}

	if (request.fps <= 0.0)
	{
		ELYSIA_LOG_ERROR("animation","Register animation failed: fps is invalid: "
			<< request.animation_key);
		return false;
	}

	AnimationDefinition definition;
	definition.animation_key = request.animation_key;
	definition.atlas_key = request.atlas_key;
	definition.fps = request.fps;
	definition.loop = request.loop;
	definition.segment_index = request.segment_index;
	definition.atlas = atlas;

	_definitions[request.animation_key] = definition;
	return true;
}

bool AnimationManager::register_animations(
	const std::vector<elysia::resources::AnimationBuildRequest>& requests,
	const elysia::resources::ResourceManager& resource_manager
)
{
	for (const elysia::resources::AnimationBuildRequest& request : requests)
	{
		const elysia::resources::Atlas* atlas = resource_manager.find_atlas(request.atlas_key);
		if (!register_animation(request, atlas))
			return false;
	}

	return true;
}

const AnimationDefinition* AnimationManager::find_definition(const std::string_view& key) const
{
	std::unordered_map<std::string, AnimationDefinition>::const_iterator iterator =
		_definitions.find(std::string(key));
	if (iterator == _definitions.end())
		return nullptr;

	return &iterator->second;
}

std::unique_ptr<Animation> AnimationManager::create_animation(const std::string_view& key) const
{
	const AnimationDefinition* definition = find_definition(key);
	if (!definition)
	{
		ELYSIA_LOG_ERROR("animation","Create animation failed: definition does not exist: "
			<< key);
		return nullptr;
	}

	std::unique_ptr<Animation> animation = std::make_unique<Animation>();
	animation->set_atlas(definition->atlas);
	animation->set_loop(definition->loop);
	animation->set_interval_seconds(1.0 / definition->fps);
	return animation;
}

}
