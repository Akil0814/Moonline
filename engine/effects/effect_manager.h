#pragma once

#include "animation_effect.h"
#include "../resources/resource_types.h"
#include "../tools/singleton.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace elysia::scene
{
class Scene;
}

namespace elysia::effects
{
enum class EffectAnchor
{
	TopLeft,
	TopCenter,
	TopRight,
	CenterLeft,
	Center,
	CenterRight,
	BottomLeft,
	BottomCenter,
	BottomRight
};

struct AnimationEffectDefinition
{
	std::string effect_key;
	std::string animation_key;
	double angle_degrees = 0.0;
	elysia::core::Vector2 default_size;
};

struct AnimationEffectSpawnRequest
{
	std::string effect_key;
	// World-space position of the selected playback anchor.
	elysia::core::Vector2 position;
	EffectAnchor anchor = EffectAnchor::TopLeft;
	std::optional<elysia::core::Vector2> size;
	std::optional<double> angle_degrees;
	std::optional<elysia::core::SpriteFlip> flip;
	double start_delay_seconds = 0.0;
};

class EffectManager : public elysia::tools::Singleton<EffectManager>
{
	friend elysia::tools::Singleton<EffectManager>;

public:
	bool register_animation_effect(const elysia::resources::AnimationEffectBuildRequest& request);
	bool register_animation_effect(const std::vector<elysia::resources::AnimationEffectBuildRequest>& requests);

	const AnimationEffectDefinition* find_animation_effect_definition(const std::string_view& key) const;
	std::unique_ptr<AnimationEffect> create_animation_effect(const AnimationEffectSpawnRequest& request) const;
	AnimationEffect* spawn_animation_effect(
		elysia::scene::Scene& scene,
		const AnimationEffectSpawnRequest& request
	) const;

private:
	std::unordered_map<std::string, AnimationEffectDefinition> _animation_effect_definitions;
};

}
