#include "user_settings_store.h"

#include "../bootstrap/bootstrap_error_utils.h"
#include "../io/json/json_loader.h"

#include <fstream>

namespace elysia::config
{
namespace
{
using RuntimeSettings = elysia::bootstrap::RuntimeSettings;
using Json = elysia::io::json;

void append_error(std::string& error,const std::string& message)
{
    elysia::bootstrap::append_bootstrap_error(error,message);
}

bool read_positive_int(const Json& node,const char* key,int& out,std::string& error)
{
    if (!node.contains(key)) return true;
    if (!node.at(key).is_number_integer() || node.at(key).get<int>() <= 0)
    {
        append_error(error,std::string("User setting must be a positive integer: ") + key);
        return false;
    }
    out = node.at(key).get<int>();
    return true;
}

bool read_positive_double(const Json& node,const char* key,double& out,std::string& error)
{
    if (!node.contains(key)) return true;
    if (!node.at(key).is_number() || node.at(key).get<double>() <= 0.0)
    {
        append_error(error,std::string("User setting must be positive: ") + key);
        return false;
    }
    out = node.at(key).get<double>();
    return true;
}

bool read_bool(const Json& node,const char* key,bool& out,std::string& error)
{
    if (!node.contains(key)) return true;
    if (!node.at(key).is_boolean())
    {
        append_error(error,std::string("User setting must be boolean: ") + key);
        return false;
    }
    out = node.at(key).get<bool>();
    return true;
}

bool read_volume(const Json& node,const char* key,int& out,std::string& error)
{
    if (!node.contains(key)) return true;
    if (!node.at(key).is_number_integer() || node.at(key).get<int>() < 0 || node.at(key).get<int>() > 100)
    {
        append_error(error,std::string("User setting volume must be within 0..100: ") + key);
        return false;
    }
    out = node.at(key).get<int>();
    return true;
}

bool read_string(const Json& node,const char* key,std::string& out,std::string& error)
{
    if (!node.contains(key)) return true;
    if (!node.at(key).is_string() || node.at(key).get<std::string>().empty())
    {
        append_error(error,std::string("User setting must be a non-empty string: ") + key);
        return false;
    }
    out = node.at(key).get<std::string>();
    return true;
}

Json serialize(const RuntimeSettings& settings)
{
    return Json{
        {"window",{{"width",settings.window_width},{"height",settings.window_height},{"fullscreen",settings.fullscreen}}},
        {"render",{{"fps",settings.target_fps},{"vsync",settings.vsync}}},
        {"audio",{{"master_volume",settings.audio.master_volume},{"music_volume",settings.audio.music_volume},{"sound_volume",settings.audio.sound_volume}}},
        {"localization",{{"language",settings.language}}}
    };
}

bool apply_overrides(const std::filesystem::path& path,RuntimeSettings& settings,std::string& error)
{
    elysia::io::JsonLoader loader;
    const auto result = loader.open_file(path);
    if (!result) { append_error(error,result.error); return false; }
    const Json& root = loader.root();
    if (!root.is_object()) { append_error(error,"User settings root must be an object."); return false; }

    const auto object = [&root,&error](const char* key,const Json*& out)
    {
        if (!root.contains(key)) return true;
        if (!root.at(key).is_object()) { append_error(error,std::string("User setting field must be an object: ") + key); return false; }
        out = &root.at(key); return true;
    };
    const Json* node = nullptr;
    if (!object("window",node)) return false;
    if (node && (!read_positive_int(*node,"width",settings.window_width,error) || !read_positive_int(*node,"height",settings.window_height,error) || !read_bool(*node,"fullscreen",settings.fullscreen,error))) return false;
    node = nullptr;
    if (!object("render",node)) return false;
    if (node && (!read_positive_double(*node,"fps",settings.target_fps,error) || !read_bool(*node,"vsync",settings.vsync,error))) return false;
    node = nullptr;
    if (!object("audio",node)) return false;
    if (node && (!read_volume(*node,"master_volume",settings.audio.master_volume,error) || !read_volume(*node,"music_volume",settings.audio.music_volume,error) || !read_volume(*node,"sound_volume",settings.audio.sound_volume,error))) return false;
    node = nullptr;
    if (!object("localization",node)) return false;
    return !node || read_string(*node,"language",settings.language,error);
}
}

std::expected<UserSettingsLoadResult,UserSettingsFailure> UserSettingsStore::load(const std::filesystem::path& path,const RuntimeSettings& defaults) const
{
    RuntimeSettings settings = defaults;
    UserSettingsLoadResult load_result;
    if (!std::filesystem::exists(path))
    {
        const auto save_result = save(path,settings);
        if (!save_result) return std::unexpected(save_result.error());
        load_result.settings = std::move(settings);
        return load_result;
    }

    std::string error;
    if (!apply_overrides(path,settings,error))
    {
        const auto save_result = save(path,defaults);
        if (!save_result) return std::unexpected(save_result.error());
        load_result.settings = defaults;
        load_result.warning = error;
        load_result.rebuilt_user_config = true;
        return load_result;
    }
    load_result.settings = std::move(settings);
    return load_result;
}

std::expected<void,UserSettingsFailure> UserSettingsStore::save(const std::filesystem::path& path,const RuntimeSettings& settings) const
{
    try { std::filesystem::create_directories(path.parent_path()); }
    catch (const std::filesystem::filesystem_error& exception)
    {
        return std::unexpected(UserSettingsFailure{UserSettingsError::SaveFailed,{},"Create settings directory failed: " + std::string(exception.what())});
    }
    std::ofstream output(path);
    if (!output.is_open())
        return std::unexpected(UserSettingsFailure{UserSettingsError::SaveFailed,{},"Open user settings for write failed: " + path.string()});
    output << serialize(settings).dump(2);
    if (!output.good())
        return std::unexpected(UserSettingsFailure{UserSettingsError::SaveFailed,{},"Write user settings failed: " + path.string()});
    return {};
}
}
