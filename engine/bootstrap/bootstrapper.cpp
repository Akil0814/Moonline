#include "bootstrapper.h"

#include "bootstrap_error_utils.h"
#include "../config/user_config_service.h"
#include "../io/loaders/content_registry_loader.h"
#include "../io/path/path_manager.h"

#include <utility>

namespace elysia::bootstrap
{
namespace
{
constexpr const char* USER_CONFIG_FILE_NAME = "user_config.json";
}

StartupParseResult Bootstrapper::parse_runtime_settings()
{
    return parse_runtime_settings({});
}

StartupParseResult Bootstrapper::parse_runtime_settings(
    const std::filesystem::path& executable_path)
{
    _startup_preload_loader.reset();
    elysia::config::UserConfigService::instance()->shutdown();

    StartupParseResult result;

    elysia::io::PathManager* path_manager = elysia::io::PathManager::instance();
    if (!path_manager->init(executable_path))
    {
        append_bootstrap_error(result.error, "Bootstrapper phase1 failed: path manager init failed.");
        return result;
    }

    if (!path_manager->ensure_runtime_dirs())
    {
        append_bootstrap_error(result.error, "Bootstrapper phase1 failed: ensure runtime dirs failed.");
        return result;
    }

	elysia::io::ContentRegistry content_registry;
    elysia::io::ContentRegistryLoader content_registry_loader;
    if (!content_registry_loader.load(path_manager->content_registry(), content_registry))
    {
        append_bootstrap_error(
            result.error,
            "Bootstrapper phase1 failed: content registry load failed."
        );
		return result;
	}

	const auto app_config_result = _app_config_loader.load(content_registry.bootstrap.app_config);
	if (!app_config_result)
	{
		result.error = app_config_result.error().message;
		return result;
	}
    const std::filesystem::path user_config_path =
        path_manager->player_data() / USER_CONFIG_FILE_NAME;

    const auto config_result =
        elysia::config::UserConfigService::instance()->initialize(
            app_config_result->user_defaults,
            user_config_path
        );
    if (!config_result)
    {
        result.error = config_result.error().message;
        return result;
    }

    if (!config_result->warning.empty())
    {
        result.warning = config_result->warning;
    }

    static_cast<UserConfigData&>(result.startup_settings) = config_result->settings;
    result.startup_settings.window_title = app_config_result->window_title;
    result.i18n_manifest_path = content_registry.required.i18n;
    result.rebuilt_user_config = config_result->rebuilt_user_config;
	result.migrated_user_config = config_result->migrated;
	result.recovered_user_config = config_result->recovered;
	_startup_preload_loader.set_manifest_path(content_registry.bootstrap.preload_manifest);
	result.content_registry = std::move(content_registry);
    result.success = true;
    return result;
}

bool Bootstrapper::preload_startup_resources(SDL_Renderer* renderer)
{
    return _startup_preload_loader.load(renderer);
}

void Bootstrapper::release_preload_textures() noexcept
{
    _startup_preload_loader.release_textures();
}

SDL_Texture* Bootstrapper::get_preload_texture(std::string_view key)
{
    return _startup_preload_loader.get_texture(key);
}
}
