#pragma once

#include "app_config_loader.h"
#include "runtime_settings.h"
#include "startup_preload_loader.h"
#include "../tools/singleton.h"

#include <SDL.h>

#include <filesystem>
#include <string_view>

namespace elysia::bootstrap
{
class Bootstrapper : public elysia::tools::Singleton<Bootstrapper>
{
    friend elysia::tools::Singleton<Bootstrapper>;

public:
    StartupParseResult parse_runtime_settings();
    StartupParseResult parse_runtime_settings(const std::filesystem::path& executable_path);
    bool preload_startup_resources(SDL_Renderer* renderer);
    void release_preload_textures() noexcept;
    SDL_Texture* get_preload_texture(std::string_view key);

private:
    AppConfigLoader _app_config_loader;
    StartupPreloadLoader _startup_preload_loader;
};
}
