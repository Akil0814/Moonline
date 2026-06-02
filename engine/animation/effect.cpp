#include "effect.h"

Effect::Effect(EffectId id,Animation* animaiton_playing) :_id(id),_animation(animaiton_playing), 															GameObject(DepthLayer::Effect)
{
	_animation->set_loop(false);
}

void Effect::on_render(SDL_Renderer* renderer)
{
	_animation->render(renderer,GameObject::rect(),_angle_degrees);
}

void Effect::on_update(double delta)
{
	_animation->update(delta);
	if (_animation->is_finished())
		GameObject::destroy();
}

std::unique_ptr<Effect> Effect::clone() const
{
	return nullptr;
}
