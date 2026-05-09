#pragma once
#include "scene_type.h"
#include "../../framework/scene/scene.h"
#include "../../framework/base/singleton.h"

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <type_traits>

class SceneFactory : public Singleton<SceneFactory>
{
	friend Singleton<SceneFactory>;
public:
	template<typename T, typename... Args>
	std::shared_ptr<T> get_scene(Args&&... args);

	bool destroy_scene(std::type_index);

	bool destroy_all_scene();

private:
	std::unordered_map<std::type_index, std::shared_ptr<Scene>> _scene_cache;
};