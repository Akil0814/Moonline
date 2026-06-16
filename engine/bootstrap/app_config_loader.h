#pragma once

#include "runtime_settings.h"

#include <filesystem>
#include <string>

class AppConfigLoader
{
public:
    struct Result
    {
        bool success = false;
        RuntimeSettings runtime_settings;
        std::filesystem::path preload_manifest_path;
        std::string error;

        explicit operator bool() const { return success; }
    };

    Result load(const std::filesystem::path& app_config_path) const;
};
