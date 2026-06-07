#include "scene_registry.h"
#include "scene_keys.h"

#include "../../engine/scene/scene_manager.h"

#include "../../gameplay/scene/loading_scene.h"
#include "../../gameplay/scene/main_menu_scene.h"

void register_all_scenes(SceneManager& scene_manager)
{
    scene_manager.register_scene<LoadingScene>(AppSceneKeys::StartupLoading);
    scene_manager.register_scene<MainMenuScene>(AppSceneKeys::MainMenu);
}
