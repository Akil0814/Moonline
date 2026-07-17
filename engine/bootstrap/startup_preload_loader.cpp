#include "startup_preload_loader.h"

#include "startup_preload_contract.h"
#include "../io/json/json_duplicate_key_checker.h"
#include "../resources/texture/surface_loader.h"
#include "../resources/texture/texture_loader.h"
#include "../io/path/path_manager.h"
#include "../resources/pipeline/resource_key_builder.h"
#include "../tools/logger.h"

#include <unordered_set>
#include <utility>

namespace elysia::bootstrap
{
void StartupPreloadLoader::set_manifest_path(const std::filesystem::path& preload_manifest_path)
{
    reset();
    _manifest_path = preload_manifest_path;
}

void StartupPreloadLoader::reset()
{
    release_textures();
    _manifest_path.clear();
    _manifest_loader.reset();
    _project_textures.clear();
}

void StartupPreloadLoader::release_textures() noexcept
{
    _texture_cache.clear();
    _renderer = nullptr;
    _is_loaded = false;
}

bool StartupPreloadLoader::load(SDL_Renderer* renderer)
{
    if (_is_loaded && renderer == _renderer)
        return true;

    if (!renderer)
    {
        ELYSIA_LOG_ERROR("bootstrap","Bootstrapper phase2 failed: renderer is null.");
        return false;
    }

    if (_manifest_path.empty())
    {
        ELYSIA_LOG_ERROR("bootstrap","Bootstrapper phase2 failed: preload manifest path is not prepared.");
        return false;
    }

    if (!load_manifest())
        return false;

    BootstrapTextureCache prepared_cache;
    if (!load_textures(renderer,prepared_cache))
        return false;

    _texture_cache = std::move(prepared_cache);
    _renderer = renderer;
    _is_loaded = true;
    return true;
}

SDL_Texture* StartupPreloadLoader::get_texture(std::string_view key) const
{
    SDL_Texture* texture = _texture_cache.find(key);
    if (!texture)
    {
        ELYSIA_LOG_WARN("bootstrap","Texture: " << key << " is not preloaded");
        return nullptr;
    }

    return texture;
}

bool StartupPreloadLoader::load_manifest()
{
    _project_textures.clear();
    if (elysia::io::has_duplicate_json_object_key(_manifest_path))
    {
        ELYSIA_LOG_ERROR("bootstrap",
            "Load preload manifest failed: duplicate JSON object key: " << _manifest_path);
        return false;
    }

    const elysia::io::JsonReadResult result = _manifest_loader.open_file(_manifest_path);
    if (!result.success)
    {
        ELYSIA_LOG_ERROR("bootstrap","Load preload manifest failed: " << result.error);
        return false;
    }

    const elysia::io::json& root = _manifest_loader.root();
    if (!root.is_object() || root.size() != 1 || !root.contains("textures")
        || !root.at("textures").is_array())
    {
        ELYSIA_LOG_ERROR("bootstrap",
            "Load preload manifest failed: root must contain only a textures array.");
        return false;
    }

    std::unordered_set<std::string> keys;
    std::vector<TextureEntry> parsed_entries;
    const elysia::io::json& textures = root.at("textures");
    parsed_entries.reserve(textures.size());

    for (const elysia::io::json& node : textures)
    {
        if (!node.is_object() || node.size() != 2
            || !node.contains("key") || !node.at("key").is_string()
            || !node.contains("file") || !node.at("file").is_string())
        {
            ELYSIA_LOG_ERROR("bootstrap",
                "Load preload manifest failed: every texture must contain string key and file fields.");
            return false;
        }

        TextureEntry entry;
        entry.key = node.at("key").get<std::string>();
        entry.file = node.at("file").get<std::string>();

        std::string key_error;
        if (!elysia::resources::ResourceKeyBuilder::validate_key(entry.key,key_error))
        {
            ELYSIA_LOG_ERROR("bootstrap",
                "Load preload manifest failed: invalid texture key: " << key_error);
            return false;
        }
        if (entry.key == startup_preload::EngineLogoTextureKey)
        {
            ELYSIA_LOG_ERROR("bootstrap",
                "Load preload manifest failed: project texture uses the engine-reserved key: "
                << entry.key);
            return false;
        }
        if (!keys.insert(entry.key).second)
        {
            ELYSIA_LOG_ERROR("bootstrap",
                "Load preload manifest failed: duplicate texture key: " << entry.key);
            return false;
        }
        if (entry.file.empty() || entry.file.is_absolute())
        {
            ELYSIA_LOG_ERROR("bootstrap",
                "Load preload manifest failed: texture file must be a non-empty relative path: "
                << entry.key);
            return false;
        }

        entry.file = entry.file.lexically_normal();
        for (const std::filesystem::path& component : entry.file)
        {
            if (component == "..")
            {
                ELYSIA_LOG_ERROR("bootstrap",
                    "Load preload manifest failed: texture file escapes the preload root: "
                    << entry.key);
                return false;
            }
        }

        parsed_entries.push_back(std::move(entry));
    }

    _project_textures = std::move(parsed_entries);
    return true;
}

bool StartupPreloadLoader::load_textures(
    SDL_Renderer* renderer,
    BootstrapTextureCache& destination
)
{
    if (!load_required_engine_texture(renderer,destination))
        return false;

    for (const TextureEntry& entry : _project_textures)
    {
        // Project branding is optional. A missing or undecodable project image
        // must not prevent the engine-owned startup scene from running.
        (void)load_optional_project_texture(renderer,entry,destination);
    }

    return true;
}

bool StartupPreloadLoader::load_required_engine_texture(
    SDL_Renderer* renderer,
    BootstrapTextureCache& destination)
{
    const std::filesystem::path file =
        elysia::io::PathManager::instance()->assets()
        / std::filesystem::path(startup_preload::EngineLogoAssetPath);

    if (load_texture(
        renderer,
        startup_preload::EngineLogoTextureKey,
        file,
        destination))
    {
        return true;
    }

    ELYSIA_LOG_ERROR("bootstrap",
        "Bootstrapper phase2 failed: required Elysia startup logo could not be loaded: "
        << file);
    return false;
}

bool StartupPreloadLoader::load_optional_project_texture(
    SDL_Renderer* renderer,
    const TextureEntry& entry,
    BootstrapTextureCache& destination)
{
    const std::filesystem::path file =
        elysia::io::PathManager::instance()->preload() / entry.file;

    if (load_texture(renderer,entry.key,file,destination))
        return true;

    ELYSIA_LOG_WARN("bootstrap",
        "Optional project startup texture was skipped: key=" << entry.key
        << ", file=" << file);
    return false;
}

bool StartupPreloadLoader::load_texture(
    SDL_Renderer* renderer,
    std::string_view key,
    const std::filesystem::path& file,
    BootstrapTextureCache& destination)
{
    elysia::resources::SurfaceLoadRequest surface_request;
    surface_request._asset_key = std::string(key);
    surface_request._frame_path = file;
    surface_request._frame_index = 0;

    elysia::resources::SurfaceLoader surface_loader;
    elysia::resources::SurfaceLoadResult surface_result =
        surface_loader.load_surface(surface_request);
    if (!surface_result._success)
        return false;

    elysia::resources::TextureLoader texture_loader;
    elysia::resources::TextureLoadResult texture_result =
        texture_loader.load_texture(renderer,surface_result);
    if (!texture_result._success)
        return false;

    if (!destination.store(std::string(key),std::move(texture_result._texture)))
    {
        ELYSIA_LOG_ERROR("bootstrap",
            "Load preload texture failed: duplicate or invalid cache key: " << key);
        return false;
    }

    return true;
}

}
