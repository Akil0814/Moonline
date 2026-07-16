#pragma once

#include "bootstrap_texture_cache.h"
#include "../io/json/json_loader.h"

#include <SDL.h>

#include <filesystem>
#include <string>
#include <string_view>

namespace elysia::bootstrap
{
class StartupPreloadLoader
{
public:
    void set_manifest_path(const std::filesystem::path& preload_manifest_path);
    void reset();
    void release_textures() noexcept;

    bool load(SDL_Renderer* renderer);
    SDL_Texture* get_texture(std::string_view key) const;

private:
    bool load_manifest();
    bool load_textures(SDL_Renderer* renderer, BootstrapTextureCache& destination);

private:
    elysia::io::JsonLoader _manifest_loader;
    std::filesystem::path _manifest_path;
    BootstrapTextureCache _texture_cache;
    SDL_Renderer* _renderer = nullptr;
    bool _is_loaded = false;
};

}
