#pragma once

#include "effect.h"
#include "../tools/singleton.h"
#include "../resources/resource_types.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class AnimationManager;

struct EffectDefinition
{
	std::string _effect_key;
	std::string _animation_key;
	double angle_degrees = 0.0;
	Vector2 default_size;
};

class EffectManager : public Singleton<EffectManager>
{
	friend Singleton<EffectManager>;

public:
	bool register_effect(
		const std::vector<EffectBuildRequest>& requests,
		const AnimationManager& resource_manager
	);

	bool has_effect(const std::string_view& key) const;

	std::unique_ptr<Effect> create_effect(const std::string_view& key) const;


private:
	std::unordered_map<std::string, EffectDefinition> _definitions;

};

