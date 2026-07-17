#pragma once

#include "bootstrap_texture_cache.h"
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
    void release_textures() noexcept;

    bool load(SDL_Renderer* renderer);
    SDL_Texture* get_texture(std::string_view key) const;

private:
    struct TextureEntry
    {
        std::string key;
        std::filesystem::path file;
    };

    bool load_manifest();
    bool load_textures(SDL_Renderer* renderer, BootstrapTextureCache& destination);
    bool load_optional_project_texture(
        SDL_Renderer* renderer,
        const TextureEntry& entry,
        BootstrapTextureCache& destination
    );
    bool load_texture(
        SDL_Renderer* renderer,
        std::string_view key,
        const std::filesystem::path& file,
        BootstrapTextureCache& destination
    );

private:
    elysia::io::JsonLoader _manifest_loader;
    std::filesystem::path _manifest_path;
    std::vector<TextureEntry> _project_textures;
    BootstrapTextureCache _texture_cache;
    SDL_Renderer* _renderer = nullptr;
    bool _is_loaded = false;
};

}
