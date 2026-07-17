#pragma once

#include "../bootstrap/runtime_settings.h"

#include <optional>
#include <string>

namespace elysia::config
{
enum class UserConfigError
{
    InvalidValue,
    ChangeHandlerUnavailable,
    RuntimeApplyFailed,
    SaveFailed,
    LoadFailed
};

struct UserConfigFailure
{
    UserConfigError error = UserConfigError::InvalidValue;
    std::string setting_name;
    std::string message;
};

struct UserConfigCommitFailure
{
    UserConfigFailure cause;
    std::optional<UserConfigFailure> rollback_failure;
};

struct UserConfigRuntimeState
{
    elysia::bootstrap::UserConfigData settings;
    bool restart_required = false;
};

enum class UserConfigApplyStatus
{
    Applied,
    PendingRestart
};

struct UserConfigInitializationFailure
{
    std::string message;
};

struct UserConfigLoadResult
{
    elysia::bootstrap::UserConfigData settings;
    std::string warning;
    bool migrated = false;
    bool recovered = false;
    bool rebuilt = false;
    bool rebuilt_user_config = false;
};
}
