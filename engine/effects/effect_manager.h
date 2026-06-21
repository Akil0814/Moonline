#pragma once

#include "effect.h"
#include "../resources/resource_types.h"
#include "../tools/singleton.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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

struct EffectDefinition
{
	std::string effect_key;
	std::string animation_key;
	double angle_degrees = 0.0;
	elysia::core::Vector2 default_size;
};

struct EffectSpawnRequest
{
	std::string effect_key;
	// World-space position of the selected playback anchor.
	elysia::core::Vector2 position;
	EffectAnchor anchor = EffectAnchor::TopLeft;
	std::optional<elysia::core::Vector2> size;
	std::optional<double> angle_degrees;
	std::optional<elysia::core::SpriteFlip> flip;
};

class EffectManager : public elysia::tools::Singleton<EffectManager>
{
	friend elysia::tools::Singleton<EffectManager>;

public:
	bool register_effect(const elysia::resources::EffectBuildRequest& request);
	bool register_effect(const std::vector<elysia::resources::EffectBuildRequest>& requests);

	const EffectDefinition* find_definition(const std::string_view& key) const;
	std::unique_ptr<Effect> create_effect(const EffectSpawnRequest& request) const;

private:
	std::unordered_map<std::string, EffectDefinition> _definitions;
};

}
