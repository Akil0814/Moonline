#include "../tools/logger.h"
#include "effect_manager.h"

#include "../animation/animation_manager.h"
#include "../scene/scene.h"

#include <algorithm>
#include <cmath>
namespace elysia::effects
{
namespace
{
bool is_supported_floating_number_character(char ch) noexcept
{
	return (ch >= '0' && ch <= '9')
		|| ch == '-'
		|| ch == '.'
		|| ch == '/'
		|| ch == '%';
}

bool is_finite_vector(const elysia::core::Vector2& value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y);
}

std::optional<elysia::core::Vector2> resolve_effect_size(
	const AnimationEffectSpawnRequest& request,
	const AnimationEffectDefinition& definition,
	const elysia::animation::Animation& animation
)
{
	if (request.size.has_value())
		return request.size;

	if (!definition.default_size.is_zero())
		return definition.default_size;

	const elysia::resources::FrameInfo* frame = animation.current_frame();
	if (!frame || frame->_width <= 0 || frame->_height <= 0)
		return std::nullopt;

	return elysia::core::Vector2(
		static_cast<float>(frame->_width),
		static_cast<float>(frame->_height)
	);
}

elysia::core::Vector2 get_effect_top_left(
	const elysia::core::Vector2& anchor_position,
	const elysia::core::Vector2& size,
	EffectAnchor anchor
)
{
	switch (anchor)
	{
	case EffectAnchor::TopLeft:
		return anchor_position;

	case EffectAnchor::TopCenter:
		return elysia::core::Vector2(anchor_position.x - size.x * 0.5f, anchor_position.y);

	case EffectAnchor::TopRight:
		return elysia::core::Vector2(anchor_position.x - size.x, anchor_position.y);

	case EffectAnchor::CenterLeft:
		return elysia::core::Vector2(anchor_position.x, anchor_position.y - size.y * 0.5f);

	case EffectAnchor::Center:
		return elysia::core::Vector2(anchor_position.x - size.x * 0.5f, anchor_position.y - size.y * 0.5f);

	case EffectAnchor::CenterRight:
		return elysia::core::Vector2(anchor_position.x - size.x, anchor_position.y - size.y * 0.5f);

	case EffectAnchor::BottomLeft:
		return elysia::core::Vector2(anchor_position.x, anchor_position.y - size.y);

	case EffectAnchor::BottomCenter:
		return elysia::core::Vector2(anchor_position.x - size.x * 0.5f, anchor_position.y - size.y);

	case EffectAnchor::BottomRight:
		return elysia::core::Vector2(anchor_position.x - size.x, anchor_position.y - size.y);

	default:
		return anchor_position;
	}
}

void apply_effect_anchor(AnimationEffect& effect, const AnimationEffectSpawnRequest& request)
{
	effect.set_position(get_effect_top_left(request.position, effect.size(), request.anchor));
}
}

bool EffectManager::register_animation_effect(const std::vector<elysia::resources::AnimationEffectBuildRequest>& requests)
{
	for (const elysia::resources::AnimationEffectBuildRequest& request : requests)
	{
		if (!register_animation_effect(request))
			return false;
	}

	return true;
}

bool EffectManager::register_animation_effect(const elysia::resources::AnimationEffectBuildRequest& request)
{
	if (request.effect_key.empty())
	{
		ELYSIA_LOG_WARN("effects","Register effect failed: effect key is empty.");
		return false;
	}

	if (request.animation_key.empty())
	{
		ELYSIA_LOG_WARN("effects","Register effect failed: animation key is empty.");
		return false;
	}

	if (request.default_size.x < 0.0f || request.default_size.y < 0.0f
		|| ((request.default_size.x == 0.0f) != (request.default_size.y == 0.0f)))
	{
		ELYSIA_LOG_WARN("effects", "Register effect failed: default size must provide positive width and height.");
		return false;
	}

	if (!elysia::animation::AnimationManager::instance()->find_definition(request.animation_key))
	{
		ELYSIA_LOG_WARN("effects","Register effect failed: can't find animation definition.");
		return false;
	}

	AnimationEffectDefinition definition;
	definition.effect_key = request.effect_key;
	definition.animation_key = request.animation_key;
	definition.default_size = request.default_size;
	definition.angle_degrees = request.default_angle_degrees;

	_animation_effect_definitions[request.effect_key] = definition;
	return true;
}

const AnimationEffectDefinition* EffectManager::find_animation_effect_definition(const std::string_view& key) const
{
	std::unordered_map<std::string, AnimationEffectDefinition>::const_iterator iterator =
		_animation_effect_definitions.find(std::string(key));
	if (iterator == _animation_effect_definitions.end())
		return nullptr;

	return &iterator->second;
}

std::unique_ptr<AnimationEffect> EffectManager::create_animation_effect(const AnimationEffectSpawnRequest& request) const
{
	const AnimationEffectDefinition* definition = find_animation_effect_definition(request.effect_key);

	if (!definition)
	{
		ELYSIA_LOG_WARN("effects","Create effect failed: definition does not exist: "
			<< request.effect_key);
		return nullptr;
	}

	std::unique_ptr<elysia::animation::Animation> animation =
		elysia::animation::AnimationManager::instance()->create_animation(definition->animation_key);

	if (!animation)
	{
		ELYSIA_LOG_WARN("effects","Create effect failed: animation creation failed: "
			<< definition->animation_key);
		return nullptr;
	}

	const std::optional<elysia::core::Vector2> final_size = resolve_effect_size(request, *definition, *animation);

	std::unique_ptr<AnimationEffect> effect = std::make_unique<AnimationEffect>(
		definition->effect_key,
		definition->animation_key,
		std::move(animation)
	);

	if (final_size.has_value())
		effect->set_size(*final_size);

	apply_effect_anchor(*effect, request);

	if (request.angle_degrees.has_value())
		effect->set_angle(*request.angle_degrees);
	else
		effect->set_angle(definition->angle_degrees);

	if (request.flip.has_value())
		effect->set_flip(*request.flip);
	else
		effect->set_flip(elysia::core::SpriteFlip::None);

	effect->set_start_delay(std::max(0.0, request.start_delay_seconds));
	effect->set_on_started(request.on_started);
	effect->set_on_finished(request.on_finished);
	for (const AnimationEffectSpawnRequest::ScheduledCallbackRequest& scheduled_callback : request.scheduled_callbacks)
	{
		effect->schedule_callback(scheduled_callback.delay_seconds, scheduled_callback.callback);
	}

	return effect;
}

bool EffectManager::spawn_animation_effect(const AnimationEffectSpawnRequest& request) const
{
	if (!_active_scene)
	{
		ELYSIA_LOG_WARN("effects", "Spawn animation effect failed: there is no active scene.");
		return false;
	}

	std::unique_ptr<AnimationEffect> effect = create_animation_effect(request);
	if (!effect)
		return false;

	if (_active_scene->add_object(std::move(effect)))
		return true;

	ELYSIA_LOG_WARN("effects", "Spawn animation effect failed: active scene rejected the effect.");
	return false;
}

std::unique_ptr<FloatingNumberEffect> EffectManager::create_floating_number_effect(
	const FloatingNumberEffectSpawnRequest& request
)
{
	if (request.text.empty())
	{
		ELYSIA_LOG_WARN("effects", "Create floating number effect failed: text is empty.");
		return nullptr;
	}

	if (!is_finite_vector(request.position)
		|| !std::isfinite(request.target_height) || request.target_height <= 0.0f
		|| !std::isfinite(request.start_delay_seconds)
		|| !std::isfinite(request.time_scale) || request.time_scale < 0.0
		|| !std::isfinite(request.lifetime_seconds) || request.lifetime_seconds <= 0.0
		|| !FloatingNumberEffect::is_valid_effects(request.effects))
	{
		ELYSIA_LOG_WARN("effects", "Create floating number effect failed: request parameters are invalid.");
		return nullptr;
	}

	for (const char ch : request.text)
	{
		if (!is_supported_floating_number_character(ch))
		{
			ELYSIA_LOG_WARN("effects", "Create floating number effect failed: text contains an unsupported character.");
			return nullptr;
		}
	}

	elysia::number::DigitCache* cache = digit_cache(request.color);
	if (!cache)
	{
		ELYSIA_LOG_WARN("effects", "Create floating number effect failed: digit cache is unavailable.");
		return nullptr;
	}

	for (const char ch : request.text)
	{
		if (!cache->get_glyph(ch))
		{
			ELYSIA_LOG_WARN("effects", "Create floating number effect failed: digit glyph is unavailable.");
			return nullptr;
		}
	}

	std::unique_ptr<FloatingNumberEffect> effect = std::make_unique<FloatingNumberEffect>(
		request.text,
		cache,
		request.position,
		request.alignment,
		request.target_height,
		request.lifetime_seconds,
		request.effects,
		request.on_finished
	);
	effect->set_start_delay(std::max(0.0, request.start_delay_seconds));
	effect->set_time_scale(request.time_scale);
	return effect;
}

bool EffectManager::spawn_floating_number_effect(const FloatingNumberEffectSpawnRequest& request)
{
	if (!_active_scene)
	{
		ELYSIA_LOG_WARN("effects", "Spawn floating number effect failed: there is no active scene.");
		return false;
	}

	std::unique_ptr<FloatingNumberEffect> effect = create_floating_number_effect(request);
	if (!effect)
		return false;

	if (_active_scene->add_object(std::move(effect)))
		return true;

	ELYSIA_LOG_WARN("effects", "Spawn floating number effect failed: active scene rejected the effect.");
	return false;
}

elysia::number::DigitCache* EffectManager::digit_cache(EffectDigitColor color)
{
	return _effect_digit_cache.digit_cache(color);
}

void EffectManager::reset_digit_caches() noexcept
{
	_effect_digit_cache.reset();
}

void EffectManager::clear_content() noexcept
{
	_animation_effect_definitions.clear();
	reset_digit_caches();
}

void EffectManager::set_active_scene(elysia::scene::Scene* scene) noexcept
{
	_active_scene = scene;
}

void EffectManager::clear_active_scene(const elysia::scene::Scene* scene) noexcept
{
	if (_active_scene == scene)
		_active_scene = nullptr;
}

}
