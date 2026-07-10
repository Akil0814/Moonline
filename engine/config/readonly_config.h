#pragma once

#include "config_initialization_failure.h"
#include "config_read_failure.h"
#include "../io/json/json_loader.h"

#include <concepts>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace elysia::config
{
template<typename T>
concept ConfigReadable =
    std::same_as<T,bool> || std::same_as<T,int> || std::same_as<T,std::int64_t>
    || std::same_as<T,double> || std::same_as<T,std::string>;

class ReadonlyConfig
{
public:
    using Json = elysia::io::json;
    using JsonReference = std::reference_wrapper<const Json>;

    [[nodiscard]] std::expected<JsonReference,ConfigReadFailure> find_node(
        std::string_view document_key,std::string_view json_pointer) const;

    template<ConfigReadable T>
    [[nodiscard]] std::expected<T,ConfigReadFailure> get(
        std::string_view document_key,std::string_view json_pointer) const
    {
        const auto node_result = find_node(document_key,json_pointer);
        if (!node_result)
            return std::unexpected(node_result.error());

        try
        {
            return node_result->get().template get<T>();
        }
        catch (const Json::type_error& exception)
        {
            return std::unexpected(ConfigReadFailure{
                ConfigReadError::TypeMismatch,
                std::string(document_key),
                std::string(json_pointer),
                exception.what()
            });
        }
    }

    [[nodiscard]] bool is_initialized() const noexcept { return _initialized; }

private:
    friend class ConfigService;
    using DocumentMap = std::unordered_map<std::string,Json>;

    [[nodiscard]] std::expected<void,ConfigInitializationFailure> initialize(
        const std::filesystem::path& manifest_path);
    void shutdown() noexcept;

    DocumentMap _documents;
    bool _initialized = false;
};
}
