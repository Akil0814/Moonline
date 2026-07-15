#include "app_config_loader.h"

#include "../io/json/strict_json.h"

#include <cmath>
#include <limits>
#include <set>

namespace elysia::bootstrap
{
namespace
{
using Json = elysia::io::json;

bool exact_fields(const Json& node,std::initializer_list<std::string_view> expected,
    std::string_view path,std::string& error)
{
    if (!node.is_object()) { error = std::string(path) + " must be an object."; return false; }
    std::set<std::string> allowed;
    for (const auto key : expected) allowed.emplace(key);
    for (const auto& [key,value] : node.items())
        if (!allowed.contains(key)) { error = "Unknown AppConfig field: " + std::string(path) + "." + key; return false; }
    for (const auto& key : allowed)
        if (!node.contains(key)) { error = "Missing AppConfig field: " + std::string(path) + "." + key; return false; }
    return true;
}

bool positive_int(const Json& node,const char* key,int& value,std::string& error)
{
    const Json& item = node.at(key);
    if (!item.is_number_integer()) { error = std::string("AppConfig field must be an integer: ") + key; return false; }
    const auto parsed = item.get<std::int64_t>();
    if (parsed <= 0 || parsed > std::numeric_limits<int>::max()) { error = std::string("AppConfig field is out of range: ") + key; return false; }
    value = static_cast<int>(parsed); return true;
}

bool volume(const Json& node,const char* key,int& value,std::string& error)
{
    if (!node.at(key).is_number_integer()) { error = std::string("AppConfig volume must be an integer: ") + key; return false; }
    const auto parsed = node.at(key).get<std::int64_t>();
    if (parsed < 0 || parsed > 100) { error = std::string("AppConfig volume must be within 0..100: ") + key; return false; }
    value = static_cast<int>(parsed); return true;
}
}

std::expected<AppConfig,AppConfigLoader::Failure> AppConfigLoader::load(
    const std::filesystem::path& path) const
{
    const auto parsed = elysia::io::load_strict_json(path);
    if (!parsed) return std::unexpected(Failure{parsed.error()});
    const Json& root = *parsed;
    std::string error;
    if (!exact_fields(root,{"schema_version","window","render","audio","localization"},"root",error))
        return std::unexpected(Failure{error});
    if (!root.at("schema_version").is_number_integer() || root.at("schema_version").get<int>() != 1)
        return std::unexpected(Failure{"AppConfig schema_version must be 1."});
    if (!exact_fields(root.at("window"),{"title","width","height","fullscreen"},"window",error)
        || !exact_fields(root.at("render"),{"fps","vsync"},"render",error)
        || !exact_fields(root.at("audio"),{"master_volume","music_volume","sound_volume"},"audio",error)
        || !exact_fields(root.at("localization"),{"language"},"localization",error))
        return std::unexpected(Failure{error});

    AppConfig result;
    const Json& window = root.at("window");
    if (!window.at("title").is_string() || (result.window_title = window.at("title").get<std::string>()).empty())
        return std::unexpected(Failure{"AppConfig window.title must be a non-empty string."});
    if (!positive_int(window,"width",result.user_defaults.window_width,error)
        || !positive_int(window,"height",result.user_defaults.window_height,error))
        return std::unexpected(Failure{error});
    if (!window.at("fullscreen").is_boolean()) return std::unexpected(Failure{"AppConfig window.fullscreen must be boolean."});
    result.user_defaults.fullscreen = window.at("fullscreen").get<bool>();

    const Json& render = root.at("render");
    if (!render.at("fps").is_number()) return std::unexpected(Failure{"AppConfig render.fps must be numeric."});
    result.user_defaults.target_fps = render.at("fps").get<double>();
    if (!std::isfinite(result.user_defaults.target_fps) || result.user_defaults.target_fps <= 0.0)
        return std::unexpected(Failure{"AppConfig render.fps must be finite and positive."});
    if (!render.at("vsync").is_boolean()) return std::unexpected(Failure{"AppConfig render.vsync must be boolean."});
    result.user_defaults.vsync = render.at("vsync").get<bool>();

    const Json& audio = root.at("audio");
    if (!volume(audio,"master_volume",result.user_defaults.audio.master_volume,error)
        || !volume(audio,"music_volume",result.user_defaults.audio.music_volume,error)
        || !volume(audio,"sound_volume",result.user_defaults.audio.sound_volume,error))
        return std::unexpected(Failure{error});
    const Json& localization = root.at("localization");
    if (!localization.at("language").is_string()
        || (result.user_defaults.language = localization.at("language").get<std::string>()).empty())
        return std::unexpected(Failure{"AppConfig localization.language must be a non-empty string."});
    return result;
}
}
