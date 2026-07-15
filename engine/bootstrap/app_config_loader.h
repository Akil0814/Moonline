#pragma once

#include "runtime_settings.h"

#include <filesystem>
#include <expected>
#include <string>

namespace elysia::bootstrap
{
class AppConfigLoader
{
public:
    struct Failure
    {
        std::string message;
    };

    [[nodiscard]] std::expected<AppConfig,Failure> load(
        const std::filesystem::path& app_config_path) const;
};

}
