#include "scene_registry.h"
#include "scene_keys.h"

#include "../../engine/scene/scene_manager.h"

#include "../../gameplay/scene/startup_loading_scene.h"
#include "../../gameplay/scene/main_menu_scene.h"
#include "../../gameplay/scene/setting_scene.h"

#include "../../gameplay/scene/character_select_scene.h"

#include "../../gameplay/scene/ui_test_scene.h"
#include "../../gameplay/scene/test_scene.h"


void register_all_scenes(elysia::scene::SceneManager& scene_manager)
{
    scene_manager.register_scene<arcneco::scene::StartupLoadingScene>(AppSceneKeys::StartupLoading);
    scene_manager.register_scene<arcneco::scene::MainMenuScene>(AppSceneKeys::MainMenu);
    scene_manager.register_scene<arcneco::scene::SettingScene>(AppSceneKeys::Setting);

    scene_manager.register_scene<arcneco::scene::CharacterSelectScene>(AppSceneKeys::CharacterSelect);

    scene_manager.register_scene<arcneco::scene::UiTestScene>(AppSceneKeys::UiTest);
    scene_manager.register_scene<arcneco::scene::TestScene>(AppSceneKeys::Test);

}
