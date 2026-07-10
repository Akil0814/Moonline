#pragma once

#include <string>

namespace elysia::config
{
enum class UserSettingsError
{
    InvalidValue,
    ChangeHandlerUnavailable,
    RuntimeApplyFailed,
    SaveFailed,
    LoadFailed
};

struct UserSettingsFailure
{
    UserSettingsError error = UserSettingsError::InvalidValue;
    std::string setting_name;
    std::string message;
};
}
