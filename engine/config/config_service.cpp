#include "config_service.h"

namespace elysia::config
{
std::expected<UserSettingsLoadResult,ConfigInitializationFailure> ConfigService::initialize(
    const elysia::bootstrap::RuntimeSettings& default_settings,
    const std::filesystem::path& user_settings_path,
    const std::filesystem::path& config_manifest_path)
{
    shutdown();
    const auto settings_result = _user_settings_store.load(user_settings_path,default_settings);
    if (!settings_result)
        return std::unexpected(ConfigInitializationFailure{settings_result.error().message});
    const auto documents_result = _readonly_config.initialize(config_manifest_path);
    if (!documents_result)
    {
        _readonly_config.shutdown();
        _user_settings.reset();
        return std::unexpected(documents_result.error());
    }
    _user_settings.initialize(settings_result->settings);
    _user_settings_path = user_settings_path;
    _initialized = true;
    return *settings_result;
}

std::expected<void,UserSettingsFailure> ConfigService::save_user_settings()
{
    if (!_initialized)
        return std::unexpected(UserSettingsFailure{UserSettingsError::SaveFailed,{},"Config service is not initialized."});
    const auto result = _user_settings_store.save(_user_settings_path,_user_settings.snapshot());
    if (!result) return std::unexpected(result.error());
    _user_settings.mark_persisted();
    return {};
}

void ConfigService::register_settings_change_handler(ISettingsChangeHandler& handler) noexcept { _user_settings.register_change_handler(handler); }
void ConfigService::unregister_settings_change_handler(ISettingsChangeHandler& handler) noexcept { _user_settings.unregister_change_handler(handler); }
void ConfigService::shutdown() noexcept { _readonly_config.shutdown(); _user_settings.reset(); _user_settings_path.clear(); _initialized = false; }
}
