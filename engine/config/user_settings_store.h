#pragma once

#include "../bootstrap/runtime_settings.h"
#include "user_settings_failure.h"

#include <expected>
#include <filesystem>
#include <string>

namespace elysia::config
{
struct UserSettingsLoadResult
{
    elysia::bootstrap::RuntimeSettings settings;
    std::string warning;
    bool rebuilt_user_config = false;
};

class UserSettingsStore
{
public:
    [[nodiscard]] std::expected<UserSettingsLoadResult,UserSettingsFailure> load(
        const std::filesystem::path& path,
        const elysia::bootstrap::RuntimeSettings& defaults) const;
    [[nodiscard]] std::expected<void,UserSettingsFailure> save(
        const std::filesystem::path& path,
        const elysia::bootstrap::RuntimeSettings& settings) const;
};
}
