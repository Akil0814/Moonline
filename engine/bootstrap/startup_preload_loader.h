#pragma once

#include "../io/json/json_loader.h"

#include <SDL.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace elysia::bootstrap
{
class StartupPreloadLoader
{
public:
    void set_manifest_path(const std::filesystem::path& preload_manifest_path);
    void reset();

    bool load(SDL_Renderer* renderer);
    SDL_Texture* get_texture(std::string_view key) const;

private:
    bool load_manifest();
    bool load_textures(SDL_Renderer* renderer);

private:
    elysia::io::JsonLoader _manifest_loader;
    std::filesystem::path _manifest_path;
    std::vector<std::string> _preloaded_texture_keys;
    bool _is_loaded = false;
};

}
