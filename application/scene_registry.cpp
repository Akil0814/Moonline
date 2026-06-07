#include "scene_registry.h"

#include "../engine/scene/scene_manager.h"

#include "../gameplay/scene/loading_scene.h"
#include "../gameplay/scene/main_menu_scene.h"

bool register_all_scenes(SceneManager& scene_manager)
{
    bool success = true;

    success = scene_manager.register_scene<LoadingScene>(SceneId::StartupLoading) && success;
    success = scene_manager.register_scene<TestScene>(SceneId::MainMenu) && success;

    return success;
}
