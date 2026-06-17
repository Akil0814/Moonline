#pragma once

#include "app_config_loader.h"
#include "runtime_settings.h"
#include "startup_preload_loader.h"
#include "user_config_store.h"
#include "../tools/singleton.h"

#include <SDL.h>

#include <string_view>

class Bootstrapper : public Singleton<Bootstrapper>
{
    friend Singleton<Bootstrapper>;

public:
    StartupParseResult parse_runtime_settings();
    bool preload_startup_resources(SDL_Renderer* renderer);
    SDL_Texture* get_preload_texture(std::string_view key);

private:
    AppConfigLoader _app_config_loader;
    UserConfigStore _user_config_store;
    StartupPreloadLoader _startup_preload_loader;
};
