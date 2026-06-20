#include "user_config_store.h"

#include "bootstrap_error_utils.h"
#include "../io/json/json_loader.h"

#include <filesystem>
#include <fstream>

bool UserConfigStore::read_positive_int_override(
    const json& node,
    const char* key,
    int& out,
    std::string& error
)
{
    if (!node.contains(key))
        return true;

    const json& value = node.at(key);
    if (!value.is_number_integer())
    {
        append_bootstrap_error(error, std::string("User config field must be an integer: ") + key);
        return false;
    }

    const int parsed = value.get<int>();
    if (parsed <= 0)
    {
        append_bootstrap_error(error, std::string("User config field must be positive: ") + key);
        return false;
    }

    out = parsed;
    return true;
}

bool UserConfigStore::read_positive_double_override(
    const json& node,
    const char* key,
    double& out,
    std::string& error
)
{
    if (!node.contains(key))
        return true;

    const json& value = node.at(key);
    if (!value.is_number())
    {
        append_bootstrap_error(error, std::string("User config field must be numeric: ") + key);
        return false;
    }

    const double parsed = value.get<double>();
    if (parsed <= 0.0)
    {
        append_bootstrap_error(error, std::string("User config field must be positive: ") + key);
        return false;
    }

    out = parsed;
    return true;
}

bool UserConfigStore::read_volume_override(
    const json& node,
    const char* key,
    int& out,
    std::string& error
)
{
    if (!node.contains(key))
        return true;

    const json& value = node.at(key);
    if (!value.is_number_integer())
    {
        append_bootstrap_error(error, std::string("User config field must be an integer: ") + key);
        return false;
    }

    const int parsed = value.get<int>();
    if (parsed < 0 || parsed > 100)
    {
        append_bootstrap_error(
            error,
            std::string("User config field must be within 0..100: ") + key
        );
        return false;
    }

    out = parsed;
    return true;
}

bool UserConfigStore::read_bool_override(
    const json& node,
    const char* key,
    bool& out,
    std::string& error
)
{
    if (!node.contains(key))
        return true;

    const json& value = node.at(key);
    if (!value.is_boolean())
    {
        append_bootstrap_error(error, std::string("User config field must be boolean: ") + key);
        return false;
    }

    out = value.get<bool>();
    return true;
}

bool UserConfigStore::read_non_empty_string_override(
    const json& node,
    const char* key,
    std::string& out,
    std::string& error
)
{
    if (!node.contains(key))
        return true;

    const json& value = node.at(key);
    if (!value.is_string())
    {
        append_bootstrap_error(error, std::string("User config field must be a string: ") + key);
        return false;
    }

    const std::string parsed = value.get<std::string>();
    if (parsed.empty())
    {
        append_bootstrap_error(
            error,
            std::string("User config field must be a non-empty string: ") + key
        );
        return false;
    }

    out = parsed;
    return true;
}

json UserConfigStore::make_user_config_json(const RuntimeSettings& runtime_settings)
{
    return json{
        {
            "window",
            {
                { "width", runtime_settings.window_width },
                { "height", runtime_settings.window_height },
                { "fullscreen", runtime_settings.fullscreen }
            }
        },
        {
            "render",
            {
                { "fps", runtime_settings.target_fps },
                { "vsync", runtime_settings.vsync }
            }
        },
        {
            "audio",
            {
                { "master_volume", runtime_settings.audio.master_volume },
                { "music_volume", runtime_settings.audio.music_volume },
                { "sound_volume", runtime_settings.audio.sound_volume }
            }
        },
        {
            "localization",
            {
                { "language", runtime_settings.language }
            }
        }
    };
}

UserConfigStore::Result UserConfigStore::load_or_create(
    const std::filesystem::path& user_config_path,
    const RuntimeSettings& default_settings
) const
{
    UserConfigStore::Result result;
    result.runtime_settings = default_settings;

    if (!std::filesystem::exists(user_config_path))
    {
        if (!write_user_config(user_config_path, default_settings, result.error))
            return result;

        result.success = true;
        return result;
    }

    RuntimeSettings merged_settings = default_settings;
    std::string user_config_error;
    if (!apply_overrides(user_config_path, merged_settings, user_config_error))
    {
        append_bootstrap_error(result.warning, user_config_error);
        result.rebuilt_user_config = true;

        if (!write_user_config(user_config_path, default_settings, result.error))
            return result;

        result.success = true;
        return result;
    }

    result.runtime_settings = merged_settings;
    result.success = true;
    return result;
}

bool UserConfigStore::save(
    const std::filesystem::path& user_config_path,
    const RuntimeSettings& runtime_settings,
    std::string& error
) const
{
    return write_user_config(user_config_path, runtime_settings, error);
}

bool UserConfigStore::apply_overrides(
    const std::filesystem::path& user_config_path,
    RuntimeSettings& runtime_settings,
    std::string& error
) const
{
    JsonLoader loader;
    const JsonReadResult open_result = loader.open_file(user_config_path);
    if (!open_result.success)
    {
        append_bootstrap_error(error, open_result.error);
        return false;
    }

    const json& root = loader.root();
    if (!root.is_object())
    {
        append_bootstrap_error(error, "User config root must be an object.");
        return false;
    }

    if (root.contains("window"))
    {
        const json& window_node = root.at("window");
        if (!window_node.is_object())
        {
            append_bootstrap_error(error, "User config field window must be an object.");
            return false;
        }

        if (!read_positive_int_override(
            window_node,
            "width",
            runtime_settings.window_width,
            error))
        {
            return false;
        }

        if (!read_positive_int_override(
            window_node,
            "height",
            runtime_settings.window_height,
            error))
        {
            return false;
        }

        if (!read_bool_override(
            window_node,
            "fullscreen",
            runtime_settings.fullscreen,
            error))
        {
            return false;
        }
    }

    if (root.contains("render"))
    {
        const json& render_node = root.at("render");
        if (!render_node.is_object())
        {
            append_bootstrap_error(error, "User config field render must be an object.");
            return false;
        }

        if (!read_positive_double_override(
            render_node,
            "fps",
            runtime_settings.target_fps,
            error))
        {
            return false;
        }

        if (!read_bool_override(
            render_node,
            "vsync",
            runtime_settings.vsync,
            error))
        {
            return false;
        }
    }

    if (root.contains("audio"))
    {
        const json& audio_node = root.at("audio");
        if (!audio_node.is_object())
        {
            append_bootstrap_error(error, "User config field audio must be an object.");
            return false;
        }

        if (!read_volume_override(
            audio_node,
            "master_volume",
            runtime_settings.audio.master_volume,
            error))
        {
            return false;
        }

        if (!read_volume_override(
            audio_node,
            "music_volume",
            runtime_settings.audio.music_volume,
            error))
        {
            return false;
        }

        if (!read_volume_override(
            audio_node,
            "sound_volume",
            runtime_settings.audio.sound_volume,
            error))
        {
            return false;
        }
    }

    if (root.contains("localization"))
    {
        const json& localization_node = root.at("localization");
        if (!localization_node.is_object())
        {
            append_bootstrap_error(error, "User config field localization must be an object.");
            return false;
        }

        if (!read_non_empty_string_override(
            localization_node,
            "language",
            runtime_settings.language,
            error))
        {
            return false;
        }
    }

    return true;
}

bool UserConfigStore::write_user_config(
    const std::filesystem::path& user_config_path,
    const RuntimeSettings& runtime_settings,
    std::string& error
) const
{
    try
    {
        std::filesystem::create_directories(user_config_path.parent_path());
    }
    catch (const std::filesystem::filesystem_error& exception)
    {
        append_bootstrap_error(
            error,
            "Create user config directory failed: " + std::string(exception.what())
        );
        return false;
    }

    std::ofstream output(user_config_path);
    if (!output.is_open())
    {
        append_bootstrap_error(
            error,
            "Open user config for write failed: " + user_config_path.string()
        );
        return false;
    }

    output << make_user_config_json(runtime_settings).dump(2);
    if (!output.good())
    {
        append_bootstrap_error(error, "Write user config failed: " + user_config_path.string());
        return false;
    }

    return true;
}
