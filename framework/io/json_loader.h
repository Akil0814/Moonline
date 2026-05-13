#pragma once
#include "../../thirdparty/nlohmann/json.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

using json = nlohmann::json;

class JsonLoader
{
public:
	bool bind_file_path(const std::filesystem::path& path);

    bool get_file_path_array(
        std::string_view key,
        std::vector<std::filesystem::path>& out
    ) const;


    const std::string& error() const;//

private:
    json _root;
    std::filesystem::path _current_file_path;

    mutable std::string _error;//
};