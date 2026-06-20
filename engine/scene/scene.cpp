#include "scene.h"

#include "scene_input_order.h"

#include "../core/interface/updatable.h"
#include "../core/render/sdl_render_command_executor.h"

#include "../input/contracts/raw_input_event_receiver.h"
#include "../input/contracts/raw_input_frame_receiver.h"

#include <algorithm>

namespace elysia::scene
{
namespace
{
template <typename Entry>
void erase_destroyed_entries(std::vector<Entry>& entries)
{
	std::erase_if(entries, [](const Entry& entry)
	{return !entry.object || entry.object->is_destroyed();});
}
}

void Scene::on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events)
{
	for (const RawInputFrameReceiverEntry& entry : _frame_receivers)
	{
		elysia::core::SceneObject* object = entry.object;

		if (!object || object->is_destroyed() || !object->is_active())
			continue;

		if (_paused && !object->receive_input_when_paused())
			continue;

		entry.receiver->on_raw_input_frame(input);
	}

	for (const elysia::input::RawInputEvent& input_event : events)
	{
		for (const RawInputEventReceiverEntry& entry : _event_receivers)
		{
			elysia::core::SceneObject* object = entry.object;

			if (!object || object->is_destroyed() || !object->is_active())
				continue;

			if (_paused && !object->receive_input_when_paused())
				continue;

			if (entry.receiver->on_raw_input_event(input_event))
				break;
		}
	}
}

void Scene::on_update(double delta)
{
	for (const UpdatableEntry& entry : _updatables)
	{
		elysia::core::SceneObject* object = entry.object;

		if (!object || object->is_destroyed() || !object->is_active())
			continue;

		if (_paused && !object->update_when_paused())
			continue;

		entry.updatable->update(delta);
	}

	remove_destroyed_objects();
}

void Scene::on_render(SDL_Renderer* renderer)
{
	if (!renderer)
		return;

	std::vector<elysia::core::RenderCommand> render_commands;
	std::vector<elysia::core::UiRenderCommand> ui_render_commands;
	render_commands.reserve(256);
	ui_render_commands.reserve(256);

	for (const auto& layer : _object_layers)
	{
		render_commands.clear();

		for (const std::unique_ptr<elysia::core::GameObject>& obj : layer)
		{
			if (!obj || obj->is_destroyed() || !obj->is_visible())
				continue;

			obj->submit_render_commands(render_commands);
		}

		elysia::core::execute_render_commands(renderer, render_commands);
	}


	ui_render_commands.clear();
	for (const auto& ui_root : _ui_roots)
	{
		if (!ui_root || ui_root->is_destroyed() || !ui_root->is_visible())
			continue;

		ui_root->submit_ui_render_commands(ui_render_commands);
	}

	elysia::core::execute_render_commands(renderer, ui_render_commands);
}

void Scene::register_scene_object_interfaces(elysia::core::SceneObject* object)
{
	if (!object)
		return;

	if (elysia::core::Updatable* updatable = dynamic_cast<elysia::core::Updatable*>(object))
		_updatables.push_back(UpdatableEntry{ object,updatable });

	if (elysia::input::RawInputFrameReceiver* receiver = dynamic_cast<elysia::input::RawInputFrameReceiver*>(object))
		_frame_receivers.push_back(RawInputFrameReceiverEntry{ object,receiver });

	if (elysia::input::RawInputEventReceiver* receiver = dynamic_cast<elysia::input::RawInputEventReceiver*>(object))
	{
		scene_input_order::insert_receiver_entry_sorted(
			_event_receivers,
			RawInputEventReceiverEntry{ object,receiver }
		);
	}

	on_scene_object_registered(*object);
}

void Scene::remove_destroyed_objects()
{
	erase_destroyed_entries(_updatables);
	erase_destroyed_entries(_frame_receivers);
	erase_destroyed_entries(_event_receivers);

	for (auto& layer : _object_layers)
	{
		std::erase_if(layer, [](const std::unique_ptr<elysia::core::GameObject>& object)
			{
				return !object || object->is_destroyed();
			});
	}

	std::erase_if(_ui_roots, [](const std::unique_ptr<elysia::ui::UiElement>& object)
		{
			return !object || object->is_destroyed();
		});
}

void Scene::notify_scene_request(const SceneRequest& request)
{
	notify_observers(
		[&](SceneRequestObserver& observer)
		{
			observer.on_scene_request(request);
		}
	);
}
void Scene::request_scene_switch(
	SceneKey target,
	const ScenePayload& payload,
	SceneReloadMode reload_mode
)
{
	SceneRequest request;
	request.type = SceneRequestType::Switch;
	request.target = target;
	request.payload = payload;
	request.reload_mode = reload_mode;

	notify_scene_request(request);
}

void Scene::request_quit()
{
	SceneRequest request;
	request.type = SceneRequestType::Quit;

	notify_scene_request(request);
}

void Scene::on_scene_object_registered(elysia::core::SceneObject& object)
{
	(void)object;
}

bool Scene::add_game_object(std::unique_ptr<elysia::core::GameObject> object)
{
	if (!object)
		return false;

	const size_t layer_index = static_cast<size_t>(object->depth_layer());

	if (layer_index >= _object_layers.size())
		return false;

	std::vector<std::unique_ptr<elysia::core::GameObject>>& layer = _object_layers[layer_index];

	auto iter = std::upper_bound(
		layer.begin(),
		layer.end(),
		object->order_in_layer(),
		[](int order, const std::unique_ptr<elysia::core::GameObject>& existing)
		{
			return order < existing->order_in_layer();
		}
	);

	layer.insert(iter, std::move(object));
	return true;
}

bool Scene::add_ui_root(std::unique_ptr<elysia::ui::UiElement> object)
{
	if (!object)
		return false;

	auto iter = std::upper_bound(
		_ui_roots.begin(),
		_ui_roots.end(),
		object->order(),
		[](int order, const std::unique_ptr<elysia::ui::UiElement>& existing)
		{
			return order < existing->order();
		}
	);

	_ui_roots.insert(iter, std::move(object));
	return true;
}

}
