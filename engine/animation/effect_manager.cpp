#include "effect_manager.h"
bool EffectManager::register_effect(
	const std::vector<EffectBuildRequest>& requests,
	const AnimationManager& resource_manager
)
{
	return true;
};

bool EffectManager::has_effect(const std::string_view& key) const
{
	return true;
};

std::unique_ptr<Effect> EffectManager::create_effect(const std::string_view& key) const
{
	return nullptr;
};