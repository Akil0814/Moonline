#pragma once

#include <SDL.h>
#include <vector>
#include <memory>
#include <array>
#include <algorithm>

#include "../game_object.h"
#include "../base/depth_layer.h"


enum class SceneType;

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void on_enter() = 0;
	virtual void on_exit() = 0;

	virtual void on_update(double delta)
	{
		for (auto& layer : _layers)
		{
			for (auto& obj : layer)
			{
				if (!obj || !obj->is_active())
					continue;

				if (_paused && !obj->will_update_when_paused())
					continue;

				obj->on_update(delta);
			}
		}
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		for (auto& layer : _layers)
		{
			for (auto& obj : layer)
			{
				if (!obj || !obj->is_visible())
					continue;

				obj->on_render(renderer);
			}
		}
	}

	virtual void on_input(const SDL_Event& event)
	{
		if (_paused)
			return;

		for (auto& layer : _layers)
		{
			for (auto& obj : layer)
			{
				if (!obj || !obj->is_active())
					continue;

				obj->on_input(event);
			}
		}
	}

	virtual void reset() = 0;

	void add_object(std::shared_ptr<GameObject> obj)
	{
		_layers[to_index(obj->depth_layer())].push_back(obj);
	}

	bool remove_object(const std::shared_ptr<GameObject>& target)
	{
		if (!target)
			return false;

		bool removed = false;

		for (auto& layer : _layers)
		{
			auto old_size = layer.size();

			layer.erase(
				std::remove(layer.begin(), layer.end(), target),
				layer.end()
			);

			if (layer.size() != old_size)
				removed = true;
		}

		return removed;
	}

	void remove_destroyed_objects()
	{
		for (auto& layer : _layers)
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

	void pause() { _paused = true; }
	void resume() { _paused = false; }

protected:
	static constexpr size_t to_index(DepthLayer layer)
	{
		return static_cast<size_t>(layer);
	}


	void sort_layers_if_dirty()
	{
		if (!_render_order_dirty)
			return;

		for (auto& layer : _layers)
		{
			std::sort(layer.begin(), layer.end(),
				[](const auto& a, const auto& b)
				{
					return a->order_in_layer() < b->order_in_layer();
				});
		}

		_render_order_dirty = false;
	}

protected:
	bool _paused = false;
	bool _render_order_dirty = true;

	std::array<
		std::vector<std::shared_ptr<GameObject>>, 
		static_cast<size_t>(DepthLayer::Count)
	> _layers;
};

