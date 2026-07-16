#include "user_config_service.h"

#include "user/user_config_store.h"

#include <memory>

namespace elysia::config
{
UserConfigService::UserConfigService() = default;

std::expected<UserConfigLoadResult,UserConfigInitializationFailure> UserConfigService::initialize(
    const elysia::bootstrap::UserConfigData& default_settings,
    const std::filesystem::path& user_config_path)
{
    shutdown();
    _user_config_store = std::make_unique<UserConfigStore>();
    const auto settings_result = _user_config_store->load(user_config_path,default_settings);
    if (!settings_result)
        return std::unexpected(UserConfigInitializationFailure{settings_result.error().message});
    _user_config.initialize(settings_result->settings);
    _user_config_path = user_config_path;
    _initialized = true;
    return *settings_result;
}

std::expected<void,UserConfigFailure> UserConfigService::save_user_config()
{
    if (!_initialized)
        return std::unexpected(UserConfigFailure{UserConfigError::SaveFailed,{},"Config service is not initialized."});
    const auto result = _user_config_store->save(_user_config_path,_user_config.snapshot());
    if (!result) return std::unexpected(result.error());
    _user_config.mark_persisted();
    return {};
}

void UserConfigService::register_user_config_change_handler(IUserConfigChangeHandler& handler) noexcept { _user_config.register_change_handler(handler); }
void UserConfigService::unregister_user_config_change_handler(IUserConfigChangeHandler& handler) noexcept { _user_config.unregister_change_handler(handler); }
void UserConfigService::shutdown() noexcept { _user_config.reset(); _user_config_store.reset(); _user_config_path.clear(); _initialized = false; }
UserConfigService::~UserConfigService() = default;
}
