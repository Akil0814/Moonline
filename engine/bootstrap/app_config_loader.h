#pragma once

#include "runtime_settings.h"

#include <filesystem>
#include <string>

namespace elysia::bootstrap
{
class AppConfigLoader
{
public:
    struct Result
    {
        bool success = false;
        RuntimeSettings runtime_settings;
		std::string error;

        explicit operator bool() const { return success; }
    };

    Result load(const std::filesystem::path& app_config_path) const;
};

}
