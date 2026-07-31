#pragma once
#include "../../core/game_object.h"
#include "../../core/interface/updatable.h"
#include "../../input/contracts/raw_input_event_receiver.h"
#include "../../physics/contracts/collider_provider.h"
#include "../../animation/animation.h"

#include <string>
#include <memory>

namespace elysia::builtin
{
class EngineCharacter : public elysia::core::GameObject,
	public elysia::input::RawInputEventReceiver,
	public elysia::core::Updatable
{
public:
	EngineCharacter();
	~EngineCharacter();
	
	void update(double delta)override;
	bool on_raw_input_event(const elysia::input::RawInputEvent& event)override;
	void submit_render_commands(std::vector<elysia::core::RenderCommand>& out_commands) const override;
	std::span<const elysia::physics::Collider> colliders() const noexcept override;

	bool set_animation(std::string idle_key,std::string move_key);


private:
	std::unique_ptr<elysia::animation::Animation> _idle;
	std::unique_ptr<elysia::animation::Animation> _move;
};

}