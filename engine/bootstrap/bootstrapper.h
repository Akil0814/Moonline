#pragma once

#include "app_config_loader.h"
#include "runtime_settings.h"
#include "startup_preload_loader.h"
#include "../tools/singleton.h"

#include <SDL.h>

#include <string_view>

namespace elysia::bootstrap
{
class Bootstrapper : public elysia::tools::Singleton<Bootstrapper>
{
    friend elysia::tools::Singleton<Bootstrapper>;

public:
    StartupParseResult parse_runtime_settings();
    bool preload_startup_resources(SDL_Renderer* renderer);
    SDL_Texture* get_preload_texture(std::string_view key);

private:
    AppConfigLoader _app_config_loader;
    StartupPreloadLoader _startup_preload_loader;
};
}
