#include "moonline_game_module.h"

#include "../scene/character_select_scene.h"
#include "../scene/main_menu_scene.h"
#include "../scene/moonline_scene_keys.h"
#include "../scene/test_scene.h"
#include "../scene/ui_test_scene.h"

#include "../../engine/scene/builtin/startup_loading_scene.h"
#include "../../engine/scene/scene_manager.h"

namespace moonline::application
{
elysia::application::ApplicationDescriptor MoonlineGameModule::descriptor() const
{
    using elysia::scene::SceneReloadMode;
    using elysia::scene::SceneRoute;
    using elysia::scene::builtin::StartupLoadingScenePayload;
    using elysia::scene::builtin::StartupLogoSlot;

    elysia::application::ApplicationDescriptor descriptor;
    descriptor.logical_width = 1280;
    descriptor.logical_height = 720;
    descriptor.initial_route = SceneRoute{
        .target = elysia::scene::builtin::StartupLoading,
        .payload = StartupLoadingScenePayload{
            .success_route = SceneRoute{
                .target = MoonlineSceneKeys::MainMenu,
                .payload = arcneco::scene::MainMenuEnterPayload{
                    .replay_theme_music = true
                },
                .reload_mode = SceneReloadMode::Reuse
            },
            .failure_route = std::nullopt,
            .project_logo = StartupLogoSlot{
                .texture_key = "moonline.brand.logo",
                .fade_in_seconds = 1.0,
                .hold_seconds = 1.0,
                .fade_out_seconds = 1.0
            },
            .wait_for_confirmation = true
        },
        .reload_mode = SceneReloadMode::Reuse
    };
    return descriptor;
}

void MoonlineGameModule::register_scenes(elysia::scene::SceneManager& scene_manager) const
{
    scene_manager.register_game_scene<arcneco::scene::MainMenuScene>(
        MoonlineSceneKeys::MainMenu);
    scene_manager.register_game_scene<arcneco::scene::CharacterSelectScene>(
        MoonlineSceneKeys::CharacterSelect);
    scene_manager.register_game_scene<arcneco::scene::UiTestScene>(
        MoonlineSceneKeys::UiTest);
    scene_manager.register_game_scene<arcneco::scene::TestScene>(
        MoonlineSceneKeys::Test);
}
}
