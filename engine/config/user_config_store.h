#pragma once

#include "../bootstrap/runtime_settings.h"
#include "user_config_failure.h"

#include <expected>
#include <filesystem>
#include <string>

namespace elysia::config
{
struct UserConfigLoadResult
{
    elysia::bootstrap::UserConfigData settings;
    std::string warning;
    bool migrated = false;
    bool recovered = false;
    bool rebuilt = false;
    bool rebuilt_user_config = false;
};

class UserConfigStore
{
public:
    [[nodiscard]] std::expected<UserConfigLoadResult,UserConfigFailure> load(
        const std::filesystem::path& path,
        const elysia::bootstrap::UserConfigData& defaults) const;
    [[nodiscard]] std::expected<void,UserConfigFailure> save(
        const std::filesystem::path& path,
        const elysia::bootstrap::UserConfigData& settings) const;
};
}
