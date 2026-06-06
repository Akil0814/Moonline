#pragma once

#include <SDL.h>

#include <algorithm>
#include <array>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/depth_layer.h"
#include "../core/game_object.h"
#include "../ui/core/ui_element.h"

struct InputEvent;
struct InputSnapshot;

struct InputSnapshotReceiver;
struct InputEventReceiver;
struct Updatable;


class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void on_enter() = 0;
	virtual void on_exit() = 0;
	virtual void reset() = 0;

	virtual void on_update(double delta);

	virtual void on_render(SDL_Renderer* renderer);

	virtual void on_input(const InputSnapshot& input,const std::vector<InputEvent>& events);

	void pause() { _paused = true; }
	void resume() { _paused = false; }
	[[nodiscard]] bool is_paused() const;

	template <typename T, typename... Args>
	T* create_and_add_object(Args&&... args)
	{
		static_assert(
			std::is_base_of_v<GameObject, T> || std::is_base_of_v<UiElement, T>,
			"T must derive from GameObject or UiElement.");

		return add_object(
			std::make_unique<T>(std::forward<Args>(args)...)
		);
	}

	template <typename T>
	T* add_object(std::unique_ptr<T> object)
	{
		static_assert(
			std::is_base_of_v<SceneObject, T>,
			"T must derive from SceneObject.");

		static_assert(
			std::is_base_of_v<GameObject, T> || std::is_base_of_v<UiElement, T>,
			"T must derive from GameObject or UiElement.");

		if (!object)
			return nullptr;

		T* raw_object = object.get();

		register_scene_object_interfaces(raw_object);

		if constexpr (std::is_base_of_v<GameObject, T>)
		{
			add_game_object(std::move(object));
		}
		else if constexpr (std::is_base_of_v<UiElement, T>)
		{
			add_ui_root(std::move(object));
		}

		return raw_object;
	}

private:
	void register_scene_object_interfaces(SceneObject* object);

protected:
	bool _paused = false;

private:
	void remove_destroyed_objects();
	void remove_destroyed_dispatch_entries();
	void add_game_object(std::unique_ptr<GameObject> object);
	void add_ui_root(std::unique_ptr<UiElement> object);

	struct UpdatableEntry
	{
		SceneObject* object = nullptr;
		Updatable* updatable = nullptr;
	};

	struct InputSnapshotReceiverEntry
	{
		SceneObject* object = nullptr;
		InputSnapshotReceiver* receiver = nullptr;
	};

	struct InputEventReceiverEntry
	{
		SceneObject* object = nullptr;
		InputEventReceiver* receiver = nullptr;
	};

private:

	std::array<std::vector<std::unique_ptr<GameObject>>,
		static_cast<size_t>(DepthLayer::Count)> _object_layers;
	std::vector<std::unique_ptr<UiElement>> _ui_roots;

	std::vector<UpdatableEntry> _updatables;
	std::vector<InputSnapshotReceiverEntry> _snapshot_receivers;
	std::vector<InputEventReceiverEntry> _event_receivers;
};
