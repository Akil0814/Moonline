#pragma once

#include "animation.h"
#include "../core/game_object.h"
#include "../tools/vector2.h"

#include <memory>

using EffectId = std::string;
class Effect : public GameObject
{
public:
	Effect(EffectId id, Animation* animaiton);
	~Effect() override = default;

	void on_render(SDL_Renderer* renderer)override;
	void on_update(double delta)override;

	std::unique_ptr<Effect> clone() const;

	void set_angle(const Vector2& target_position, double angle_degrees);

	bool bind_animation(Animation* animaiton);

private:
	EffectId _id;
	double _angle_degrees = 0.0;
	Animation* _animation = nullptr;
};
