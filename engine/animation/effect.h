#pragma once

#include "animation.h"
#include "../core/game_object.h"
#include "../core/interface/updatable.h"

#include <memory>

namespace elysia::animation
{
class Effect : public elysia::core::GameObject, public elysia::core::Updatable
{
public:
	Effect(std::string effect_key, std::string animation_key, std::unique_ptr<Animation> animation);
	~Effect() override = default;

	void submit_render_commands(std::vector<elysia::core::RenderCommand>& out_commands) const override;
	void update(double delta) override;

	std::unique_ptr<Effect> clone() const;

	void set_angle(double angle_degrees);
	void set_flip(elysia::core::SpriteFlip flip);

private:
	std::string _effect_key;
	std::string _animation_key;
	double _angle_degrees = 0.0;
	elysia::core::SpriteFlip _flip = elysia::core::SpriteFlip::None;
	std::unique_ptr<Animation> _animation;
};

}
