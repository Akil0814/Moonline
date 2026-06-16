#include "app_config_loader.h"

#include "bootstrap_error_utils.h"
#include "../io/json/json_loader.h"
#include "../io/path/path_manager.h"

#include <filesystem>

namespace
{
constexpr const char* DEFAULT_PRELOAD_MANIFEST_PATH = "preload/preload_manifest.json";
}

AppConfigLoader::Result AppConfigLoader::load(const std::filesystem::path& app_config_path) const
{
    AppConfigLoader::Result result;

    JsonLoader loader;
    const JsonReadResult open_result = loader.open_file(app_config_path);
    if (!open_result.success)
    {
        append_bootstrap_error(result.error, open_result.error);
        return result;
    }

    const json* window_node = nullptr;
    if (!loader.get_object("window", window_node))
    {
        append_bootstrap_error(result.error, "App config is missing window object.");
        return result;
    }

    if (!loader.get(*window_node, "title", result.runtime_settings.window_title))
    {
        append_bootstrap_error(result.error, "App config is missing window.title.");
        return result;
    }

    if (!loader.get(*window_node, "default_width", result.runtime_settings.window_width)
        || result.runtime_settings.window_width <= 0)
    {
        append_bootstrap_error(result.error, "App config default_width is missing or invalid.");
        return result;
    }

    if (!loader.get(*window_node, "default_height", result.runtime_settings.window_height)
        || result.runtime_settings.window_height <= 0)
    {
        append_bootstrap_error(result.error, "App config default_height is missing or invalid.");
        return result;
    }

    if (!loader.get(*window_node, "fullscreen", result.runtime_settings.fullscreen))
    {
        append_bootstrap_error(result.error, "App config fullscreen is missing or invalid.");
        return result;
    }

    const json* render_node = nullptr;
    if (!loader.get_object("render", render_node))
    {
        append_bootstrap_error(result.error, "App config is missing render object.");
        return result;
    }

    if (!loader.get(*render_node, "default_fps", result.runtime_settings.target_fps)
        || result.runtime_settings.target_fps <= 0.0)
    {
        append_bootstrap_error(result.error, "App config default_fps is missing or invalid.");
        return result;
    }

    if (!loader.get(*render_node, "vsync", result.runtime_settings.vsync))
    {
        append_bootstrap_error(result.error, "App config vsync is missing or invalid.");
        return result;
    }

    const json* startup_node = nullptr;
    std::filesystem::path preload_manifest_relative = DEFAULT_PRELOAD_MANIFEST_PATH;
    const json& root = loader.root();
    if (root.contains("startup"))
    {
        if (!loader.get_object("startup", startup_node))
        {
            append_bootstrap_error(result.error, "App config startup must be an object.");
            return result;
        }

        if (startup_node->contains("preload_manifest"))
        {
            const json& preload_manifest_node = startup_node->at("preload_manifest");
            if (!preload_manifest_node.is_string())
            {
                append_bootstrap_error(
                    result.error,
                    "App config startup.preload_manifest must be a string."
                );
                return result;
            }

            preload_manifest_relative = preload_manifest_node.get<std::string>();
        }
    }

    result.preload_manifest_path =
        PathManager::instance()->resolve_asset_path(preload_manifest_relative);

    if (!std::filesystem::exists(result.preload_manifest_path))
    {
        append_bootstrap_error(
            result.error,
            "App config preload manifest does not exist: " + result.preload_manifest_path.string()
        );
        return result;
    }

    result.success = true;
    return result;
}
