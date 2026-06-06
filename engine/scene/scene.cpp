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
	{return !entry.object || entry.object->is_destroyed();});
}

template <typename Entry>
void insert_input_event_entry_sorted(std::vector<Entry>& entries, Entry entry)
{
	auto iter = std::upper_bound(entries.begin(),entries.end(),entry,
		[](const Entry& lhs, const Entry& rhs)
		{	return scene_object_input_event_less(lhs.object, rhs.object);}
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

		if (_paused && !object->update_when_paused())
			continue;

		entry.updatable->update(delta);
	}

	remove_destroyed_objects();
}

void Scene::on_render(SDL_Renderer* renderer)
{
	std::vector<RenderCommand> render_commands;
	std::vector<UiRenderCommand> ui_render_commands;
	render_commands.reserve(256);
	ui_render_commands.reserve(256);

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


	ui_render_commands.clear();
	for (const auto& ui_root : _ui_roots)
	{
		if (!ui_root || ui_root->is_destroyed() || !ui_root->is_visible())
			continue;

		ui_root->submit_ui_render_commands(ui_render_commands);
	}

	execute_render_commands(renderer, ui_render_commands);
}

void Scene::register_scene_object_interfaces(SceneObject* object)
{
	if (!object)
		return;

	if (Updatable* updatable = dynamic_cast<Updatable*>(object))
		_updatables.push_back(UpdatableEntry{ object,updatable });

	if (InputSnapshotReceiver* receiver =dynamic_cast<InputSnapshotReceiver*>(object))
		_snapshot_receivers.push_back(InputSnapshotReceiverEntry{ object,receiver });

	if (InputEventReceiver* receiver =dynamic_cast<InputEventReceiver*>(object))
		insert_input_event_entry_sorted(_event_receivers,InputEventReceiverEntry{object,receiver});
}

void Scene::remove_destroyed_objects()
{
	erase_destroyed_entries(_updatables);
	erase_destroyed_entries(_snapshot_receivers);
	erase_destroyed_entries(_event_receivers);

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

void Scene::add_game_object(std::unique_ptr<GameObject> object)
{
	if (!object)
		return;

	const size_t layer_index = static_cast<size_t>(object->depth_layer());

	if (layer_index >= _object_layers.size())
		return;

	std::vector<std::unique_ptr<GameObject>>& layer = _object_layers[layer_index];

	auto iter = std::upper_bound(
		layer.begin(),layer.end(),object->order_in_layer(),
		[](const std::unique_ptr<GameObject>& existing, int order)
		{	return existing->order_in_layer() < order;}
	);

	layer.insert(iter, std::move(object));
}

void Scene::add_ui_root(std::unique_ptr<UiElement> object)
{
	if (!object)
		return;

	auto iter = std::upper_bound(_ui_roots.begin(), _ui_roots.end(), object->order(),
		[](int order, const std::unique_ptr<UiElement>& existing)
		{	return order < existing->order();}
	);

	_ui_roots.insert(iter, std::move(object));
}
