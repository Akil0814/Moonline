#include "bootstrapper.h"

#include "bootstrap_error_utils.h"
#include "../config/config_manager.h"
#include "../io/loaders/assets_structure_loader.h"
#include "../io/path/path_manager.h"

namespace elysia::bootstrap
{
namespace
{
constexpr const char* APP_CONFIG_PATH = "configs/global/app_config.elysia::io::json";
constexpr const char* USER_CONFIG_FILE_NAME = "user_config.elysia::io::json";
}

StartupParseResult Bootstrapper::parse_runtime_settings()
{
    _startup_preload_loader.reset();
    elysia::config::ConfigManager::instance()->shutdown();

    StartupParseResult result;

    elysia::io::PathManager* path_manager = elysia::io::PathManager::instance();
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
        path_manager->to_config_path(APP_CONFIG_PATH);
    const AppConfigLoader::Result app_config_result =
        _app_config_loader.load(app_config_path);
    if (!app_config_result.success)
    {
        result.error = app_config_result.error;
        return result;
    }

    elysia::io::AssetManifestPaths manifest_paths;
    elysia::io::AssetsStructureLoader assets_structure_loader;
    if (!assets_structure_loader.load(path_manager->assets_structure(), manifest_paths))
    {
        append_bootstrap_error(
            result.error,
            "Bootstrapper phase1 failed: assets structure load failed."
        );
        return result;
    }

    const std::filesystem::path user_config_path =
        path_manager->player_data() / USER_CONFIG_FILE_NAME;

    const elysia::config::ConfigInitResult config_result =
        elysia::config::ConfigManager::instance()->init(
            app_config_result.runtime_settings,
            user_config_path
        );
    if (!config_result.success)
    {
        result.error = config_result.error;
        return result;
    }

    if (!config_result.warning.empty())
    {
        result.warning = config_result.warning;
    }

    result.runtime_settings = config_result.runtime_settings;
    result.i18n_manifest_path = manifest_paths.i18n;
    result.rebuilt_user_config = config_result.rebuilt_user_config;
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
}
