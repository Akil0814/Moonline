#include "effect_manager.h"
#include "animation_manager.h"

#include <iostream>

bool EffectManager::register_effect(const std::vector<EffectBuildRequest>& requests)
{
	for (const EffectBuildRequest& request : requests)
	{
		if (!register_effect(request))
			return false;
	}

	return true;
};

bool EffectManager::register_effect(const EffectBuildRequest& request)
{
	if (request.effect_key.empty())
	{
		std::cout << "Register effect failed: effect key is empty." << std::endl;
		return false;
	}

	if (request.animation_key.empty())
	{
		std::cout << "Register effect failed: animation key is empty." << std::endl;
		return false;
	}

	if (!AnimationManager::instance()->find_definition(request.animation_key))
	{
		std::cout << "Register effect failed: can't find animation definition." << std::endl;
		return false;
	}


	EffectDefinition definition;
	definition.effect_key = request.effect_key;
	definition.animation_key = request.animation_key;

	_definitions[request.effect_key] = definition;
	return true;
};

const EffectDefinition* EffectManager::find_definition(const std::string_view& key) const
{
	std::unordered_map<std::string, EffectDefinition>::const_iterator iterator =
		_definitions.find(std::string(key));
	if (iterator == _definitions.end())
		return nullptr;

	return &iterator->second;
}

std::unique_ptr<Effect> EffectManager::create_effect(const EffectSpawnRequest& request) const
{
	const EffectDefinition* definition = find_definition(request.effect_key);

	if (!definition)
	{
		std::cout << "Create effect failed: definition does not exist: "
			<< request.effect_key << std::endl;
		return nullptr;
	}

	std::unique_ptr<Animation> animation =
		AnimationManager::instance()->create_animation(definition->animation_key);

	if (!animation)
	{
		std::cout << "Create effect failed: animation creation failed: "
			<< definition->animation_key << std::endl;
		return nullptr;
	}

	std::unique_ptr<Effect> effect = std::make_unique<Effect>(
		definition->effect_key,
		definition->animation_key,
		std::move(animation)
	);

	effect->set_position(request.position);

	if (request.size.has_value())
		effect->set_size(*request.size);
	else if (!definition->default_size.is_zero())
		effect->set_size(definition->default_size);

	if (request.angle_degrees.has_value())
		effect->set_angle(*request.angle_degrees);
	else
		effect->set_angle(definition->angle_degrees);

	if (request.flip.has_value())
		effect->set_flip(*request.flip);
	else
		effect->set_flip(SpriteFlip::None);

	return effect;
}

