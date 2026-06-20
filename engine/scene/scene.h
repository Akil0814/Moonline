#pragma once

#include <SDL.h>

#include <array>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "scene_request.h"
#include "scene_request_observer.h"

#include "../core/depth_layer.h"
#include "../core/game_object.h"
#include "../core/event/subject.h"
#include "../core/interface/updatable.h"
#include "../input/contracts/raw_input_event_receiver.h"
#include "../input/contracts/raw_input_frame_receiver.h"
#include "../input/raw_input_frame.h"
#include "../input/raw_input_types.h"
#include "../ui/core/ui_element.h"

namespace elysia::scene
{
class Scene : public elysia::core::Subject<SceneRequestObserver>
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;

	virtual void on_enter(const ScenePayload& payload) = 0;
	virtual void on_exit() = 0;
	virtual void reset() = 0;

	virtual void on_update(double delta);

	virtual void on_render(SDL_Renderer* renderer);

	virtual void on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events);

	void pause() { _paused = true; }
	void resume() { _paused = false; }
	[[nodiscard]] bool is_paused()const { return _paused; }

	template <typename T, typename... Args>
	T* create_and_add_object(Args&&... args)
	{
		static_assert(
			std::is_base_of_v<elysia::core::GameObject, T> || std::is_base_of_v<elysia::ui::UiElement, T>,
			"T must derive from elysia::core::GameObject or elysia::ui::UiElement.");

		return add_object(
			std::make_unique<T>(std::forward<Args>(args)...)
		);
	}

	template <typename T>
	T* add_object(std::unique_ptr<T> object)
	{
		static_assert(
			std::is_base_of_v<elysia::core::SceneObject, T>,
			"T must derive from elysia::core::SceneObject.");

		static_assert(
			std::is_base_of_v<elysia::core::GameObject, T> || std::is_base_of_v<elysia::ui::UiElement, T>,
			"T must derive from elysia::core::GameObject or elysia::ui::UiElement.");

		if (!object)
			return nullptr;

		T* raw_object = object.get();
		bool added = false;

		if constexpr (std::is_base_of_v<elysia::core::GameObject, T>)
			added = add_game_object(std::move(object));
		else if constexpr (std::is_base_of_v<elysia::ui::UiElement, T>)
			added = add_ui_root(std::move(object));

		if (!added)
			return nullptr;

		register_scene_object_interfaces(raw_object);

		return raw_object;
	}

protected:
	void notify_scene_request(const SceneRequest& request);
	void request_scene_switch(
		SceneKey target,
		const ScenePayload& payload = {},
		SceneReloadMode reload_mode = SceneReloadMode::Reuse
	);
	void request_quit();
	virtual void on_scene_object_registered(elysia::core::SceneObject& object);

protected:
	bool _paused = false;

private:
	void register_scene_object_interfaces(elysia::core::SceneObject* object);
	void remove_destroyed_objects();
	bool add_game_object(std::unique_ptr<elysia::core::GameObject> object);
	bool add_ui_root(std::unique_ptr<elysia::ui::UiElement> object);

	struct UpdatableEntry
	{
		elysia::core::SceneObject* object = nullptr;
		elysia::core::Updatable* updatable = nullptr;
	};

	struct RawInputFrameReceiverEntry
	{
		elysia::core::SceneObject* object = nullptr;
		elysia::input::RawInputFrameReceiver* receiver = nullptr;
	};

	struct RawInputEventReceiverEntry
	{
		elysia::core::SceneObject* object = nullptr;
		elysia::input::RawInputEventReceiver* receiver = nullptr;
	};

private:

	std::array<std::vector<std::unique_ptr<elysia::core::GameObject>>,
		static_cast<size_t>(elysia::core::DepthLayer::Count)> _object_layers;
	std::vector<std::unique_ptr<elysia::ui::UiElement>> _ui_roots;

	std::vector<UpdatableEntry> _updatables;
	std::vector<RawInputFrameReceiverEntry> _frame_receivers;
	std::vector<RawInputEventReceiverEntry> _event_receivers;
};

}
