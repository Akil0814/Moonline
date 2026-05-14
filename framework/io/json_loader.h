#pragma once
#include "../../thirdparty/nlohmann/json.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

using json = nlohmann::json;

struct JsonReadResult
{
    bool success = false;
    std::string error;
};

class JsonLoader
{
public:
    JsonReadResult open_file(const std::filesystem::path& path);
    void reset();

    bool is_loaded() const;
    const json& root() const;

    template<typename T>
    JsonReadResult get(std::string_view key, T& out) const;

    template<typename T>
    JsonReadResult get(
        const json& node,
        std::string_view key,
        T& out
    ) const;

    template<typename T>
    JsonReadResult get_array(std::string_view key, std::vector<T>& out) const;

    template<typename T>
    JsonReadResult get_array(
        const json& node,
        std::string_view key,
        std::vector<T>& out
    ) const;

    template<typename T>
    T get_or(std::string_view key, T default_value) const;

    template<typename T>
    T get_or(
        const json& node,
        std::string_view key,
        T default_value
    ) const;

    JsonReadResult get_object(
        std::string_view key,
        const json*& out
    ) const;

    JsonReadResult get_object(
        const json& node,
        std::string_view key,
        const json*& out
    ) const;

private:
    void add_error_message(JsonReadResult& result,
        const std::string& error);

private:
    json _root;
    std::filesystem::path _path;
    bool _loaded = false;
};