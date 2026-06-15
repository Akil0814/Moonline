#pragma once
#include "../io/json/json_loader.h"

#include "../tools/singleton.h"
#include "../core/error.h"

#include <SDL.h>

#include <string>
#include <string_view>
#include <vector>

class ResourceBootstrapper : public Singleton<ResourceBootstrapper>
{
    friend Singleton<ResourceBootstrapper>;
public:
    bool bootstrap(SDL_Renderer* renderer);
    const std::vector<Error>& get_error_list() const;

    SDL_Texture* get_preload_texture(std::string_view key);

private:
    bool load_preload_manifest();
    bool load_preload_textures(SDL_Renderer* renderer);

private:
    JsonLoader _json_loader;

    std::vector<Error> _error_list;

    std::vector<std::string> _preloaded_texture_keys;
};
