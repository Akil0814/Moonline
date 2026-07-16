#pragma once

#include "animation_effect.h"
#include "number/floating_number_glyph_cache.h"
#include "number/floating_number_effect.h"
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
class SceneManager;
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
	struct ScheduledCallbackRequest
	{
		double delay_seconds = 0.0;
		AnimationEffect::Callback callback;
	};

	std::string effect_key;
	// World-space position of the selected playback anchor.
	elysia::core::Vector2 position;
	EffectAnchor anchor = EffectAnchor::TopLeft;
	std::optional<elysia::core::Vector2> size;
	std::optional<double> angle_degrees;
	std::optional<elysia::core::SpriteFlip> flip;
	double start_delay_seconds = 0.0;
	AnimationEffect::Callback on_started;
	AnimationEffect::Callback on_finished;
	std::vector<ScheduledCallbackRequest> scheduled_callbacks;
};

struct FloatingNumberEffectSpawnRequest
{
	std::string text;
	FloatingNumberColor color = FloatingNumberColor::White;
	elysia::core::Vector2 position;
	FloatingNumberAlignment alignment = FloatingNumberAlignment::Center;
	float target_height = 20.0f;
	double start_delay_seconds = 0.0;
	double time_scale = 1.0;
	double lifetime_seconds = 0.6;
	FloatingNumberEffects effects;
	FloatingNumberEffect::Callback on_finished;
};

class EffectManager : public elysia::tools::Singleton<EffectManager>
{
	friend elysia::tools::Singleton<EffectManager>;

public:
	bool register_animation_effect(const elysia::resources::AnimationEffectBuildRequest& request);
	bool register_animation_effect(const std::vector<elysia::resources::AnimationEffectBuildRequest>& requests);

	const AnimationEffectDefinition* find_animation_effect_definition(const std::string_view& key) const;
	std::unique_ptr<AnimationEffect> create_animation_effect(const AnimationEffectSpawnRequest& request) const;
	bool spawn_animation_effect(const AnimationEffectSpawnRequest& request) const;
	std::unique_ptr<FloatingNumberEffect> create_floating_number_effect(const FloatingNumberEffectSpawnRequest& request);
	bool spawn_floating_number_effect(const FloatingNumberEffectSpawnRequest& request);
	void clear_content() noexcept;

private:
	friend class elysia::scene::SceneManager;

	void set_active_scene(elysia::scene::Scene* scene) noexcept;
	void clear_active_scene(const elysia::scene::Scene* scene) noexcept;

	std::unordered_map<std::string, AnimationEffectDefinition> _animation_effect_definitions;
	FloatingNumberGlyphCache _floating_number_glyph_cache;
	elysia::scene::Scene* _active_scene = nullptr;
};

}
