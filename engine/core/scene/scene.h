#pragma once

#include <SDL.h>
#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "../depth_layer.h"
#include "../game_object.h"
#include "../render/sdl_render_command_executor.h"

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void on_enter() = 0;
	virtual void on_exit() = 0;

	virtual void on_update(double delta)
	{
		for (auto& layer : _object_layers)
		{
			const std::vector<std::shared_ptr<GameObject>> objects = layer;
			for (const std::shared_ptr<GameObject>& obj : objects)
			{
				if (!obj || obj->is_destroyed())
					continue;

				if (!obj->is_active())
					continue;

				obj->update(obj->scaled_delta(delta));
			}
		}

		remove_destroyed_objects();
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		std::vector<RenderCommand> render_commands;
		render_commands.reserve(1);

		for (auto& layer : _object_layers)
		{
			const std::vector<std::shared_ptr<GameObject>> objects = layer;
			for (const std::shared_ptr<GameObject>& obj : objects)
			{
				if (!obj || obj->is_destroyed())
					continue;

				if (!obj->is_visible())
					continue;

				render_commands.clear();
				obj->submit_render_commands(render_commands);
				execute_render_commands(renderer, render_commands);
			}
		}
	}

	virtual void submit_render_commands(std::vector<RenderCommand>& out_commands) const
	{
		for (const auto& layer : _object_layers)
		{
			const std::vector<std::shared_ptr<GameObject>> objects = layer;
			for (const std::shared_ptr<GameObject>& obj : objects)
			{
				if (!obj || obj->is_destroyed())
					continue;

				if (!obj->is_visible())
					continue;

				obj->submit_render_commands(out_commands);
			}
		}
	}

	virtual void on_input(
		const InputSnapshot& input,
		const std::vector<InputEvent>& events
	)
	{
		for (auto& layer : _object_layers)
		{
			const std::vector<std::shared_ptr<GameObject>> objects = layer;
			for (const std::shared_ptr<GameObject>& obj : objects)
			{
				if (!obj || obj->is_destroyed())
					continue;

				if (!obj->is_active())
					continue;

				obj->on_input(input);

				for (const InputEvent& input_event : events)
					obj->on_input_event(input_event);
			}
		}
	}

	virtual void reset() = 0;

	void add_object(std::shared_ptr<GameObject> obj)
	{
		if (!obj)
			return;

		auto& layer = _object_layers[to_index(obj->depth_layer())];

		auto iter = std::lower_bound(layer.begin(), layer.end(), obj,
			[](const auto& a, const auto& b)
			{
				return a->order_in_layer() < b->order_in_layer();
			}
		);

		layer.insert(iter, obj);
	}

	void clear_objects()
	{
		for (auto& layer : _object_layers)
			layer.clear();
	}

	void pause() { _paused = true; }
	void resume() { _paused = false; }
	[[nodiscard]] bool is_paused() const { return _paused; }

protected:
	static constexpr size_t to_index(DepthLayer layer)
	{
		return static_cast<size_t>(layer);
	}

	void reset_objects()
	{
		for (auto& layer : _object_layers)
		{
			const std::vector<std::shared_ptr<GameObject>> objects = layer;
			for (const std::shared_ptr<GameObject>& obj : objects)
			{
				if (obj)
					obj->reset();
			}
		}
	}

	void remove_destroyed_objects()
	{
		for (auto& layer : _object_layers)
		{
			layer.erase(
				std::remove_if(layer.begin(), layer.end(),
					[](const auto& obj)
					{
						return !obj || obj->is_destroyed();
					}),
				layer.end()
			);
		}
	}

protected:
	bool _paused = false;

	std::array<
		std::vector<std::shared_ptr<GameObject>>,
		static_cast<size_t>(DepthLayer::Count)
	> _object_layers;
};
