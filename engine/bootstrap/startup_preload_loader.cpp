#include "startup_preload_loader.h"

#include "../resources/resource_manager.h"
#include "../resources/texture/surface_loader.h"
#include "../resources/texture/texture_loader.h"

#include <algorithm>
#include <iostream>

void StartupPreloadLoader::set_manifest_path(const std::filesystem::path& preload_manifest_path)
{
    _manifest_path = preload_manifest_path;
    _manifest_loader.reset();
    _preloaded_texture_keys.clear();
    _is_loaded = false;
}

void StartupPreloadLoader::reset()
{
    _manifest_path.clear();
    _manifest_loader.reset();
    _preloaded_texture_keys.clear();
    _is_loaded = false;
}

bool StartupPreloadLoader::load(SDL_Renderer* renderer)
{
    if (_is_loaded)
        return true;

    if (!renderer)
    {
        std::cout << "Bootstrapper phase2 failed: renderer is null." << std::endl;
        return false;
    }

    if (_manifest_path.empty())
    {
        std::cout << "Bootstrapper phase2 failed: preload manifest path is not prepared."
            << std::endl;
        return false;
    }

    if (!load_manifest())
        return false;

    if (!load_textures(renderer))
        return false;

    _is_loaded = true;
    return true;
}

SDL_Texture* StartupPreloadLoader::get_texture(std::string_view key) const
{
    const std::string requested_key(key);
    if (std::find(
        _preloaded_texture_keys.begin(),
        _preloaded_texture_keys.end(),
        requested_key) == _preloaded_texture_keys.end())
    {
        std::cout << "Texture: " << requested_key << " is not preloaded" << std::endl;
        return nullptr;
    }

    return ResourceManager::instance()->find_texture(key);
}

bool StartupPreloadLoader::load_manifest()
{
    const JsonReadResult result = _manifest_loader.open_file(_manifest_path);
    if (!result.success)
    {
        std::cout << "Load preload manifest failed: " << result.error;
        return false;
    }

    return true;
}

bool StartupPreloadLoader::load_textures(SDL_Renderer* renderer)
{
    _preloaded_texture_keys.clear();

    std::vector<std::string> texture_paths;
    const JsonReadResult array_result =
        _manifest_loader.get_array("textures", texture_paths);
    if (!array_result.success)
    {
        std::cout << "Load preload textures failed: " << array_result.error;
        return false;
    }

    ResourceManager* resource_manager = ResourceManager::instance();
    SurfaceLoader surface_loader;
    TextureLoader texture_loader;
    const std::filesystem::path preload_root = _manifest_path.parent_path();

    for (const std::string& relative_path : texture_paths)
    {
        if (relative_path.empty())
        {
            std::cout << "Load preload textures failed: texture path is empty." << std::endl;
            return false;
        }

        SurfaceLoadRequest surface_request;
        surface_request._asset_key = relative_path;
        surface_request._frame_path = preload_root / std::filesystem::path(relative_path);
        surface_request._frame_index = 0;

        SurfaceLoadResult surface_result = surface_loader.load_surface(surface_request);
        if (!surface_result._success)
            return false;

        TextureLoadResult texture_result =
            texture_loader.load_texture(renderer, surface_result);
        if (!texture_result._success)
            return false;

        if (!resource_manager->texture_manager().store_texture(
            relative_path,
            std::move(texture_result._texture)))
        {
            std::cout << "Load preload textures failed: store texture failed: "
                << relative_path << std::endl;
            return false;
        }

        _preloaded_texture_keys.push_back(relative_path);
    }

    return true;
}
