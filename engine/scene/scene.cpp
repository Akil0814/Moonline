#include "scene.h"

#include "../core/interface/updatable.h"
#include "../core/render/sdl_render_command_executor.h"

#include "../input/contracts/input_event_receiver.h"
#include "../input/contracts/input_snapshot_receiver.h"

#include <tuple>

namespace
{
template <typename Entry>
void erase_destroyed_entries(std::vector<Entry>& entries)
{
	std::erase_if(entries, [](const Entry& entry)
		{
			return !entry.object || entry.object->is_destroyed();
		});
}

[[nodiscard]] bool scene_object_dispatch_less(
	const SceneObject* lhs,
	const SceneObject* rhs
) noexcept
{
	if (lhs == rhs)
		return false;

	const GameObject* lhs_game_object = dynamic_cast<const GameObject*>(lhs);
	const GameObject* rhs_game_object = dynamic_cast<const GameObject*>(rhs);
	if (lhs_game_object && rhs_game_object)
	{
		return std::make_tuple(lhs_game_object->depth_layer(), lhs_game_object->order_in_layer())
			< std::make_tuple(rhs_game_object->depth_layer(), rhs_game_object->order_in_layer());
	}

	const UiElement* lhs_ui_element = dynamic_cast<const UiElement*>(lhs);
	const UiElement* rhs_ui_element = dynamic_cast<const UiElement*>(rhs);
	if (lhs_ui_element && rhs_ui_element)
		return lhs_ui_element->order() < rhs_ui_element->order();

	if (lhs_game_object && rhs_ui_element)
		return true;

	if (lhs_ui_element && rhs_game_object)
		return false;

	return lhs < rhs;
}

template <typename Entry>
void insert_dispatch_entry_sorted(std::vector<Entry>& entries, Entry entry)
{
	auto iter = std::lower_bound(
		entries.begin(),
		entries.end(),
		entry,
		[](const Entry& lhs, const Entry& rhs)
		{
			return scene_object_dispatch_less(lhs.object, rhs.object);
		}
	);

	entries.insert(iter, entry);
}
}

void Scene::on_update(double delta)
{
	for (const UpdatableEntry& entry : _updatables)
	{
		SceneObject* object = entry.object;
		if (!object || object->is_destroyed() || !object->is_active())
			continue;

		if (_paused)
			continue;

		entry.updatable->update(delta);
	}

	remove_destroyed_objects();
}

void Scene::on_render(SDL_Renderer* renderer)
{
	std::vector<RenderCommand> render_commands;
	std::vector<UiRenderCommand> ui_render_commands;
	render_commands.reserve(512);
	ui_render_commands.reserve(512);

	for (const auto& layer : _object_layers)
	{
		render_commands.clear();

		for (const std::unique_ptr<GameObject>& obj : layer)
		{
			if (!obj || obj->is_destroyed() || !obj->is_visible())
				continue;

			obj->submit_render_commands(render_commands);
		}

		execute_render_commands(renderer, render_commands);
	}

	for (const auto& ui_set : _ui_roots)
	{
		if (!ui_set || ui_set->is_destroyed() || !ui_set->is_visible())
			continue;

		ui_set->submit_ui_render_commands(ui_render_commands);
	}

	execute_render_commands(renderer, ui_render_commands);
}

void Scene::on_input(const InputSnapshot& input, const std::vector<InputEvent>& events)
{
	if (!_paused)
	{
		for (const InputSnapshotReceiverEntry& entry : _snapshot_receivers)
		{
			SceneObject* object = entry.object;
			if (!object || object->is_destroyed() || !object->is_active())
				continue;

			entry.receiver->on_input_snapshot(input);
		}
	}

	for (const InputEvent& input_event : events)
	{
		bool consumed = false;

		for (const InputEventReceiverEntry& entry : _event_receivers)
		{
			SceneObject* object = entry.object;
			if (!object || object->is_destroyed() || !object->is_active())
				continue;

			if (entry.receiver->on_input_event(input_event))
			{
				consumed = true;
				break;
			}
		}

		if (consumed)
			continue;
	}
}

bool Scene::is_paused() const
{
	return _paused;
}

void Scene::register_scene_object_interfaces(SceneObject* object)
{
	if (!object)
		return;

	if (Updatable* updatable = dynamic_cast<Updatable*>(object))
		insert_dispatch_entry_sorted(_updatables, UpdatableEntry{ object, updatable });

	if (InputSnapshotReceiver* receiver = dynamic_cast<InputSnapshotReceiver*>(object))
	{
		insert_dispatch_entry_sorted(
			_snapshot_receivers,
			InputSnapshotReceiverEntry{ object, receiver }
		);
	}

	if (InputEventReceiver* receiver = dynamic_cast<InputEventReceiver*>(object))
	{
		insert_dispatch_entry_sorted(
			_event_receivers,
			InputEventReceiverEntry{ object, receiver }
		);
	}
}

void Scene::remove_destroyed_objects()
{
	remove_destroyed_dispatch_entries();

	for (auto& layer : _object_layers)
	{
		std::erase_if(layer, [](const std::unique_ptr<GameObject>& object)
			{
				return !object || object->is_destroyed();
			});
	}

	std::erase_if(_ui_roots, [](const std::unique_ptr<UiElement>& object)
		{
			return !object || object->is_destroyed();
		});
}

void Scene::remove_destroyed_dispatch_entries()
{
	erase_destroyed_entries(_updatables);
	erase_destroyed_entries(_snapshot_receivers);
	erase_destroyed_entries(_event_receivers);
}

void Scene::add_game_object(std::unique_ptr<GameObject> object)
{
	if (!object)
		return;

	auto& layer = _object_layers[static_cast<size_t>(object->depth_layer())];
	auto iter = std::lower_bound(
		layer.begin(),
		layer.end(),
		object->order_in_layer(),
		[](const std::unique_ptr<GameObject>& existing, int order)
		{
			return existing && existing->order_in_layer() < order;
		}
	);

	layer.insert(iter, std::move(object));
}

void Scene::add_ui_root(std::unique_ptr<UiElement> object)
{
	if (!object)
		return;

	auto iter = std::lower_bound(
		_ui_roots.begin(),
		_ui_roots.end(),
		object->order(),
		[](const std::unique_ptr<UiElement>& existing, int order)
		{
			return existing && existing->order() < order;
		}
	);

	_ui_roots.insert(iter, std::move(object));
}
