#include "bootstrapper.h"

#include "bootstrap_error_utils.h"
#include "../io/path/path_manager.h"

namespace
{
constexpr const char* APP_CONFIG_PATH = "configs/global/app_config.json";
constexpr const char* USER_CONFIG_FILE_NAME = "user_config.json";
}

StartupParseResult Bootstrapper::parse_runtime_settings(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    _startup_preload_loader.reset();

    StartupParseResult result;

    PathManager* path_manager = PathManager::instance();
    if (!path_manager->init())
    {
        append_bootstrap_error(result.error, "Bootstrapper phase1 failed: path manager init failed.");
        return result;
    }

    if (!path_manager->ensure_runtime_dirs())
    {
        append_bootstrap_error(result.error, "Bootstrapper phase1 failed: ensure runtime dirs failed.");
        return result;
    }

    const std::filesystem::path app_config_path =
        path_manager->resolve_config_path(APP_CONFIG_PATH);
    const AppConfigLoader::Result app_config_result =
        _app_config_loader.load(app_config_path);
    if (!app_config_result.success)
    {
        result.error = app_config_result.error;
        return result;
    }

    const UserConfigStore::Result user_config_result =
        _user_config_store.load_or_create(
            path_manager->player_data() / USER_CONFIG_FILE_NAME,
            app_config_result.runtime_settings
        );
    if (!user_config_result.success)
    {
        result.error = user_config_result.error;
        return result;
    }

    if (!user_config_result.warning.empty())
    {
        result.warning = user_config_result.warning;
    }

    result.runtime_settings = user_config_result.runtime_settings;
    result.rebuilt_user_config = user_config_result.rebuilt_user_config;
    _startup_preload_loader.set_manifest_path(app_config_result.preload_manifest_path);
    result.success = true;
    return result;
}

bool Bootstrapper::preload_startup_resources(SDL_Renderer* renderer)
{
    return _startup_preload_loader.load(renderer);
}

SDL_Texture* Bootstrapper::get_preload_texture(std::string_view key)
{
    return _startup_preload_loader.get_texture(key);
}
