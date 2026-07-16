#pragma once

#include "../bootstrap/runtime_settings.h"

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
