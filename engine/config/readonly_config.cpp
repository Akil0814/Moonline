#include "readonly_config.h"

#include "../io/path/path_manager.h"

#include <filesystem>

namespace elysia::config
{
std::expected<ReadonlyConfig::JsonReference,ConfigReadFailure> ReadonlyConfig::find_node(
    std::string_view document_key,std::string_view json_pointer) const
{
    if (!_initialized)
        return std::unexpected(ConfigReadFailure{ConfigReadError::NotInitialized,{}, {},"Readonly config is not initialized."});

    const auto document_it = _documents.find(std::string(document_key));
    if (document_it == _documents.end())
        return std::unexpected(ConfigReadFailure{ConfigReadError::DocumentNotFound,std::string(document_key),std::string(json_pointer),"Config document was not found."});

    if (json_pointer.empty())
        return std::cref(document_it->second);

    try
    {
        const Json::json_pointer pointer{std::string(json_pointer)};
        if (!document_it->second.contains(pointer))
            return std::unexpected(ConfigReadFailure{ConfigReadError::PathNotFound,std::string(document_key),std::string(json_pointer),"JSON pointer does not exist."});
        return std::cref(document_it->second.at(pointer));
    }
    catch (const Json::parse_error& exception)
    {
        return std::unexpected(ConfigReadFailure{ConfigReadError::InvalidJsonPointer,std::string(document_key),std::string(json_pointer),exception.what()});
    }
    catch (const Json::out_of_range& exception)
    {
        return std::unexpected(ConfigReadFailure{ConfigReadError::PathNotFound,std::string(document_key),std::string(json_pointer),exception.what()});
    }
}

std::expected<void,ConfigInitializationFailure> ReadonlyConfig::initialize(const std::filesystem::path& manifest_path)
{
    shutdown();
    elysia::io::JsonLoader manifest_loader;
    const auto open_result = manifest_loader.open_file(manifest_path);
    if (!open_result)
        return std::unexpected(ConfigInitializationFailure{"Open config document manifest failed: " + manifest_path.string() + "; " + open_result.error});

    const Json& root = manifest_loader.root();
    if (!root.is_object() || !root.contains("documents") || !root.at("documents").is_object())
        return std::unexpected(ConfigInitializationFailure{"Config document manifest must contain an object field documents: " + manifest_path.string()});

    DocumentMap documents;
    for (auto it = root.at("documents").begin(); it != root.at("documents").end(); ++it)
    {
        const std::string key = it.key();
        if (key.empty() || !it.value().is_string() || it.value().get<std::string>().empty())
            return std::unexpected(ConfigInitializationFailure{"Config document manifest has an empty key or path: " + manifest_path.string()});

        const std::string relative_path = it.value().get<std::string>();
        const std::filesystem::path absolute_path = elysia::io::PathManager::instance()->to_asset_path(relative_path);
        if (!std::filesystem::is_regular_file(absolute_path))
            return std::unexpected(ConfigInitializationFailure{"Config document not found; key=" + key + ", relative=" + relative_path + ", absolute=" + absolute_path.string()});

        elysia::io::JsonLoader document_loader;
        const auto document_result = document_loader.open_file(absolute_path);
        if (!document_result)
            return std::unexpected(ConfigInitializationFailure{"Config document load failed; key=" + key + ", relative=" + relative_path + ", absolute=" + absolute_path.string() + "; " + document_result.error});

        if (!documents.emplace(key,document_loader.root()).second)
            return std::unexpected(ConfigInitializationFailure{"Duplicate config document key: " + key});
    }

    _documents = std::move(documents);
    _initialized = true;
    return {};
}

void ReadonlyConfig::shutdown() noexcept
{
    _documents.clear();
    _initialized = false;
}
}
