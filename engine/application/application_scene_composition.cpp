#include "application_scene_composition.h"

#include "../scene/builtin/settings_scene.h"
#include "../scene/builtin/startup_loading_scene.h"
#include "../scene/builtin/ui_test_scene.h"
#include "../scene/builtin/engine_feature_test_scene.h"
#include "../scene/scene_manager.h"

namespace elysia::application
{
ApplicationDescriptor describe_game_module(const IGameModule& game_module)
{
    return game_module.descriptor();
}

void compose_application_scenes(
    elysia::scene::SceneManager& scene_manager,
    const IGameModule& game_module,
    const ApplicationDescriptor& descriptor)
{
    scene_manager.register_builtin_scene<
        elysia::scene::builtin::StartupLoadingScene>(
            elysia::scene::builtin::StartupLoading);
    scene_manager.register_builtin_scene<
        elysia::scene::builtin::SettingsScene>(
            elysia::scene::builtin::Settings);
    scene_manager.register_builtin_scene<
        elysia::scene::builtin::UiTestScene>(
            elysia::scene::builtin::UiTest);
    scene_manager.register_builtin_scene<
        elysia::scene::builtin::EngineFeatureTestScene>(
            elysia::scene::builtin::EngineFeatureTest);

    game_module.register_scenes(scene_manager);
    scene_manager.start(descriptor.initial_route);
}
}
