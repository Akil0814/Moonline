#include "scene_factory.h"

namespace elysia::scene
{
SceneFactory::~SceneFactory()
{
	destroy_all_scene();
}

bool SceneFactory::destroy_all_scene()
{
	_scene_cache.clear();
	return true;
}

}
