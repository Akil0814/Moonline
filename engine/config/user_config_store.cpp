#include "user_config_store.h"

#include "../io/json/strict_json.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>

namespace elysia::config
{
namespace
{
using Data = elysia::bootstrap::UserConfigData;
using Json = elysia::io::json;
constexpr int k_schema_version = 1;

enum class ParseKind { ValidV1, ValidV0, Invalid, Future };
struct ParseResult { ParseKind kind = ParseKind::Invalid; Data data; std::string error; };

bool allowed_fields(const Json& object,std::initializer_list<std::string_view> fields,
    bool require_all,std::string_view path,std::string& error)
{
    if (!object.is_object()) { error = std::string(path) + " must be an object."; return false; }
    std::set<std::string> allowed;
    for (auto field : fields) allowed.emplace(field);
    for (const auto& [key,value] : object.items())
        if (!allowed.contains(key)) { error = "Unknown UserConfig field: " + std::string(path) + "." + key; return false; }
    if (require_all) for (const auto& key : allowed)
        if (!object.contains(key)) { error = "Missing UserConfig field: " + std::string(path) + "." + key; return false; }
    return true;
}

bool positive_int(const Json& node,const char* key,int& out,std::string& error,bool required)
{
    if (!node.contains(key)) return !required;
    if (!node.at(key).is_number_integer()) { error = std::string(key) + " must be an integer."; return false; }
    const auto value = node.at(key).get<std::int64_t>();
    if (value <= 0 || value > std::numeric_limits<int>::max()) { error = std::string(key) + " is out of range."; return false; }
    out = static_cast<int>(value); return true;
}

bool boolean(const Json& node,const char* key,bool& out,std::string& error,bool required)
{
    if (!node.contains(key)) return !required;
    if (!node.at(key).is_boolean()) { error = std::string(key) + " must be boolean."; return false; }
    out = node.at(key).get<bool>(); return true;
}

bool volume(const Json& node,const char* key,int& out,std::string& error,bool required)
{
    if (!node.contains(key)) return !required;
    if (!node.at(key).is_number_integer()) { error = std::string(key) + " must be an integer."; return false; }
    const auto value = node.at(key).get<std::int64_t>();
    if (value < 0 || value > 100) { error = std::string(key) + " must be within 0..100."; return false; }
    out = static_cast<int>(value); return true;
}

ParseResult parse(const std::filesystem::path& path,const Data& defaults)
{
    const auto loaded = elysia::io::load_strict_json(path);
    if (!loaded) return {ParseKind::Invalid,{},loaded.error()};
    const Json& root = *loaded;
    if (!root.is_object()) return {ParseKind::Invalid,{},"UserConfig root must be an object."};
    const bool v1 = root.contains("schema_version");
    if (v1)
    {
        if (!root.at("schema_version").is_number_integer()) return {ParseKind::Invalid,{},"UserConfig schema_version must be an integer."};
        const int version = root.at("schema_version").get<int>();
        if (version > k_schema_version) return {ParseKind::Future,{},"UserConfig schema_version is newer than this application."};
        if (version != k_schema_version) return {ParseKind::Invalid,{},"Unsupported UserConfig schema_version."};
    }
    Data data = defaults;
    std::string error;
    if (!allowed_fields(root,v1
            ? std::initializer_list<std::string_view>{"schema_version","window","render","audio","localization"}
            : std::initializer_list<std::string_view>{"window","render","audio","localization"},v1,"root",error))
        return {ParseKind::Invalid,{},error};
    auto section = [&](const char* name,const Json*& out)
    {
        if (!root.contains(name)) return !v1;
        if (!root.at(name).is_object()) { error = std::string(name) + " must be an object."; return false; }
        out = &root.at(name); return true;
    };
    const Json* node = nullptr;
    if (!section("window",node)) return {ParseKind::Invalid,{},error};
    if (node && (!allowed_fields(*node,{"width","height","fullscreen"},v1,"window",error)
        || !positive_int(*node,"width",data.window_width,error,v1)
        || !positive_int(*node,"height",data.window_height,error,v1)
        || !boolean(*node,"fullscreen",data.fullscreen,error,v1))) return {ParseKind::Invalid,{},error};
    node = nullptr;
    if (!section("render",node)) return {ParseKind::Invalid,{},error};
    if (node)
    {
        if (!allowed_fields(*node,{"fps","vsync"},v1,"render",error)) return {ParseKind::Invalid,{},error};
        if ((v1 || node->contains("fps")))
        {
            if (!node->contains("fps") || !node->at("fps").is_number()) return {ParseKind::Invalid,{},"fps must be numeric."};
            data.target_fps = node->at("fps").get<double>();
            if (!std::isfinite(data.target_fps) || data.target_fps <= 0.0) return {ParseKind::Invalid,{},"fps must be finite and positive."};
        }
        if (!boolean(*node,"vsync",data.vsync,error,v1)) return {ParseKind::Invalid,{},error};
    }
    node = nullptr;
    if (!section("audio",node)) return {ParseKind::Invalid,{},error};
    if (node && (!allowed_fields(*node,{"master_volume","music_volume","sound_volume"},v1,"audio",error)
        || !volume(*node,"master_volume",data.audio.master_volume,error,v1)
        || !volume(*node,"music_volume",data.audio.music_volume,error,v1)
        || !volume(*node,"sound_volume",data.audio.sound_volume,error,v1))) return {ParseKind::Invalid,{},error};
    node = nullptr;
    if (!section("localization",node)) return {ParseKind::Invalid,{},error};
    if (node)
    {
        if (!allowed_fields(*node,{"language"},v1,"localization",error)) return {ParseKind::Invalid,{},error};
        if (node->contains("language"))
        {
            if (!node->at("language").is_string()) return {ParseKind::Invalid,{},"language must be a string."};
            const std::string language = node->at("language").get<std::string>();
            data.language = language.empty() && !v1 ? defaults.language : language;
            if (data.language.empty()) return {ParseKind::Invalid,{},"language must be non-empty."};
        }
        else if (v1) return {ParseKind::Invalid,{},"Missing UserConfig field: localization.language"};
    }
    return {v1 ? ParseKind::ValidV1 : ParseKind::ValidV0,std::move(data),{}};
}

Json serialize(const Data& data)
{
    return {{"schema_version",k_schema_version},
        {"window",{{"width",data.window_width},{"height",data.window_height},{"fullscreen",data.fullscreen}}},
        {"render",{{"fps",data.target_fps},{"vsync",data.vsync}}},
        {"audio",{{"master_volume",data.audio.master_volume},{"music_volume",data.audio.music_volume},{"sound_volume",data.audio.sound_volume}}},
        {"localization",{{"language",data.language}}}};
}

UserConfigFailure failure(UserConfigError kind,std::string message)
{ return {kind,{},std::move(message)}; }
}

std::expected<void,UserConfigFailure> UserConfigStore::save(const std::filesystem::path& path,const Data& data) const
{
    const auto tmp = path.string() + ".tmp";
    const auto bak = path.string() + ".bak";
    try
    {
        if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        { std::ofstream output(tmp,std::ios::trunc); if (!output) return std::unexpected(failure(UserConfigError::SaveFailed,"Open temporary UserConfig failed: " + tmp)); output << serialize(data).dump(2); if (!output.good()) return std::unexpected(failure(UserConfigError::SaveFailed,"Write temporary UserConfig failed: " + tmp)); }
        const auto verified = parse(tmp,data);
        if (verified.kind != ParseKind::ValidV1) return std::unexpected(failure(UserConfigError::SaveFailed,"Temporary UserConfig verification failed: " + verified.error));
        if (std::filesystem::exists(bak)) std::filesystem::remove(bak);
        const bool had_primary = std::filesystem::exists(path);
        if (had_primary) std::filesystem::rename(path,bak);
        try { std::filesystem::rename(tmp,path); }
        catch (...)
        {
            if (had_primary && std::filesystem::exists(bak) && !std::filesystem::exists(path)) std::filesystem::rename(bak,path);
            throw;
        }
        return {};
    }
    catch (const std::exception& exception)
    { return std::unexpected(failure(UserConfigError::SaveFailed,"Atomic UserConfig save failed: " + std::string(exception.what()))); }
}

std::expected<UserConfigLoadResult,UserConfigFailure> UserConfigStore::load(
    const std::filesystem::path& path,const Data& defaults) const
{
    UserConfigLoadResult result;
    const std::filesystem::path tmp = path.string() + ".tmp";
    const std::filesystem::path bak = path.string() + ".bak";
    if (std::filesystem::exists(path))
    {
        const auto primary = parse(path,defaults);
        if (primary.kind == ParseKind::Future) return std::unexpected(failure(UserConfigError::LoadFailed,primary.error));
        if (primary.kind == ParseKind::ValidV1 || primary.kind == ParseKind::ValidV0)
        {
            result.settings = primary.data;
            result.migrated = primary.kind == ParseKind::ValidV0;
            if (result.migrated) { if (auto saved = save(path,result.settings); !saved) return std::unexpected(saved.error()); }
            return result;
        }
        result.warning = primary.error;
        try
        {
            const auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            std::filesystem::rename(path,path.string() + "." + std::to_string(stamp) + ".corrupt");
        }
        catch (const std::exception& exception) { result.warning += " Failed to archive corrupt file: " + std::string(exception.what()); }
    }
    for (const auto& candidate : {tmp,bak})
    {
        if (!std::filesystem::exists(candidate)) continue;
        const auto recovered = parse(candidate,defaults);
        if (recovered.kind == ParseKind::Future) return std::unexpected(failure(UserConfigError::LoadFailed,recovered.error));
        if (recovered.kind == ParseKind::ValidV1 || recovered.kind == ParseKind::ValidV0)
        {
            result.settings = recovered.data; result.recovered = true; result.migrated = recovered.kind == ParseKind::ValidV0;
            if (auto saved = save(path,result.settings); !saved) return std::unexpected(saved.error());
            return result;
        }
    }
    result.settings = defaults; result.rebuilt = true; result.rebuilt_user_config = true;
    if (auto saved = save(path,result.settings); !saved) return std::unexpected(saved.error());
    return result;
}
}
