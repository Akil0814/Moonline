#define SDL_MAIN_HANDLED

#include "engine/config/config_service.h"
#include "engine/io/loaders/content_registry_loader.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/game_content_loader.h"
#include "engine/resources/resource_manager.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

namespace
{
using moonline::tests::require;
}

int main()
{
    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0,
        "game content loader config test must initialize SDL");
    require((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == IMG_INIT_PNG,
        "game content loader config test must initialize SDL_image");
    require(TTF_Init() == 0,
        "game content loader config test must initialize SDL_ttf");
    require(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0,
        "game content loader config test must open SDL_mixer audio");

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 64, 64, 32, SDL_PIXELFORMAT_RGBA32);
    require(surface != nullptr, "game content loader config test must create a target surface");
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    require(renderer != nullptr, "game content loader config test must create a software renderer");

    auto* paths = elysia::io::PathManager::instance();
    require(paths->init(), "game content loader config test must initialize PathManager");
    auto* configs = elysia::config::ConfigService::instance();
    configs->shutdown();
    elysia::resources::ResourceManager::instance()->clear();
	elysia::io::ContentRegistry content_registry;
	require(elysia::io::ContentRegistryLoader{}.load(paths->content_registry(), content_registry),
		"game content loader config test must parse the content registry once before loading");

    elysia::loading::GameContentLoader loader;
	require(loader.start(renderer, content_registry), "game content loader must start with a valid deferred config snapshot");
    require(!configs->is_initialized(),
        "ConfigService must remain unavailable while resources are still loading");

    for (int update_count = 0; loader.is_running() && update_count < 10000; ++update_count)
    {
        loader.update();
        SDL_Delay(1);
    }

    require(loader.is_finished(), "game content loader must finish with the repository content");
    require(configs->is_initialized(),
        "ConfigService must publish the deferred snapshot only after content loading finishes");

    loader.reset();
    require(!configs->is_initialized(),
        "resetting content loading must clear the published config snapshot");

    elysia::resources::ResourceManager::instance()->clear();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
