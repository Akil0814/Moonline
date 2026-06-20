#include "config_manager.h"

#include "../bootstrap/bootstrap_error_utils.h"

namespace elysia::config
{
namespace
{
constexpr const char* KEY_WINDOW_WIDTH = "window.width";
constexpr const char* KEY_WINDOW_HEIGHT = "window.height";
constexpr const char* KEY_WINDOW_FULLSCREEN = "window.fullscreen";
constexpr const char* KEY_RENDER_FPS = "render.fps";
constexpr const char* KEY_RENDER_VSYNC = "render.vsync";
constexpr const char* KEY_AUDIO_MASTER_VOLUME = "audio.master_volume";
constexpr const char* KEY_AUDIO_MUSIC_VOLUME = "audio.music_volume";
constexpr const char* KEY_AUDIO_SOUND_VOLUME = "audio.sound_volume";
constexpr const char* KEY_LOCALIZATION_LANGUAGE = "localization.language";

bool validate_positive_int(const ConfigValue& value, std::string& error)
{
    const int parsed = std::get<int>(value);
    if (parsed > 0)
        return true;

    elysia::bootstrap::append_bootstrap_error(error, "Config value must be a positive integer.");
    return false;
}

bool validate_bool(const ConfigValue& value, std::string& error)
{
    (void)value;
    (void)error;
    return true;
}

bool validate_positive_double(const ConfigValue& value, std::string& error)
{
    const double parsed = std::get<double>(value);
    if (parsed > 0.0)
        return true;

    elysia::bootstrap::append_bootstrap_error(error, "Config value must be positive.");
    return false;
}

bool validate_volume(const ConfigValue& value, std::string& error)
{
    const int parsed = std::get<int>(value);
    if (parsed >= 0 && parsed <= 100)
        return true;

    elysia::bootstrap::append_bootstrap_error(error, "Config value must be within 0..100.");
    return false;
}

bool validate_non_empty_string(const ConfigValue& value, std::string& error)
{
    const std::string& parsed = std::get<std::string>(value);
    if (!parsed.empty())
        return true;

    elysia::bootstrap::append_bootstrap_error(error, "Config value must be a non-empty string.");
    return false;
}

ConfigValue read_window_width(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.window_width;
}

ConfigValue read_window_height(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.window_height;
}

ConfigValue read_window_fullscreen(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.fullscreen;
}

ConfigValue read_render_fps(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.target_fps;
}

ConfigValue read_render_vsync(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.vsync;
}

ConfigValue read_audio_master_volume(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.audio.master_volume;
}

ConfigValue read_audio_music_volume(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.audio.music_volume;
}

ConfigValue read_audio_sound_volume(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.audio.sound_volume;
}

ConfigValue read_localization_language(const elysia::bootstrap::RuntimeSettings& settings)
{
    return settings.language;
}

void write_window_width(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.window_width = std::get<int>(value);
}

void write_window_height(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.window_height = std::get<int>(value);
}

void write_window_fullscreen(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.fullscreen = std::get<bool>(value);
}

void write_render_fps(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.target_fps = std::get<double>(value);
}

void write_render_vsync(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.vsync = std::get<bool>(value);
}

void write_audio_master_volume(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.audio.master_volume = std::get<int>(value);
}

void write_audio_music_volume(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.audio.music_volume = std::get<int>(value);
}

void write_audio_sound_volume(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.audio.sound_volume = std::get<int>(value);
}

void write_localization_language(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
{
    settings.language = std::get<std::string>(value);
}
}

ConfigInitResult ConfigManager::init(
    const elysia::bootstrap::RuntimeSettings& default_settings,
    const std::filesystem::path& user_config_path
)
{
    shutdown();

    ConfigInitResult result;

    _user_config_path = user_config_path;
    _base_runtime_settings = default_settings;
    register_runtime_settings(default_settings);

    const elysia::bootstrap::UserConfigStore::Result user_config_result =
        _user_config_store.load_or_create(_user_config_path, default_settings);
    if (!user_config_result.success)
    {
        result.error = user_config_result.error;
        shutdown();
        return result;
    }

    _base_runtime_settings = user_config_result.runtime_settings;
    load_values_from_settings(user_config_result.runtime_settings);
    _dirty = false;
    _initialized = true;

    result.success = true;
    result.runtime_settings = user_config_result.runtime_settings;
    result.warning = user_config_result.warning;
    result.rebuilt_user_config = user_config_result.rebuilt_user_config;
    return result;
}

void ConfigManager::shutdown()
{
    _user_config_path.clear();
    _entries.clear();
    _values.clear();
    _base_runtime_settings = elysia::bootstrap::RuntimeSettings{};
    _dirty = false;
    _initialized = false;
}

bool ConfigManager::save(std::string& error)
{
    if (!_initialized)
    {
        elysia::bootstrap::append_bootstrap_error(error, "Save config failed: config manager is not initialized.");
        return false;
    }

    if (!_dirty)
        return true;

    elysia::bootstrap::RuntimeSettings runtime_settings = snapshot_runtime_settings();
    if (!_user_config_store.save(_user_config_path, runtime_settings, error))
        return false;

    _base_runtime_settings = runtime_settings;
    _dirty = false;
    return true;
}

bool ConfigManager::is_dirty() const
{
    return _dirty;
}

bool ConfigManager::has_value(std::string_view key) const
{
    return get_value(key) != nullptr;
}

const ConfigValue* ConfigManager::get_value(std::string_view key) const
{
    const auto value_it = _values.find(std::string(key));
    if (value_it == _values.end())
        return nullptr;

    return &value_it->second;
}

bool ConfigManager::set_value(
    std::string_view key,
    const ConfigValue& value,
    std::string& error
)
{
    if (!_initialized)
    {
        elysia::bootstrap::append_bootstrap_error(error, "Set config value failed: config manager is not initialized.");
        return false;
    }

    const auto entry_it = _entries.find(std::string(key));
    if (entry_it == _entries.end())
    {
        elysia::bootstrap::append_bootstrap_error(error, "Set config value failed: unknown key: " + std::string(key));
        return false;
    }

    const ConfigEntry& entry = entry_it->second;
    if (!matches_type(value, entry.type))
    {
        elysia::bootstrap::append_bootstrap_error(error, "Set config value failed: type mismatch for key: " + std::string(key));
        return false;
    }

    if (entry.validator && !entry.validator(value, error))
        return false;

    ConfigValue& current_value = _values[entry_it->first];
    if (current_value == value)
        return true;

    current_value = value;
    _dirty = true;
    return true;
}

elysia::bootstrap::RuntimeSettings ConfigManager::snapshot_runtime_settings() const
{
    elysia::bootstrap::RuntimeSettings runtime_settings = _base_runtime_settings;
    for (const auto& [key, entry] : _entries)
    {
        const auto value_it = _values.find(key);
        if (value_it == _values.end() || !entry.write_to_settings)
            continue;

        entry.write_to_settings(value_it->second, runtime_settings);
    }

    return runtime_settings;
}

std::string_view ConfigManager::language() const
{
    static const std::string empty;
    const ConfigValue* value = get_value(KEY_LOCALIZATION_LANGUAGE);
    if (!value)
        return empty;

    return std::get<std::string>(*value);
}

bool ConfigManager::set_language(std::string language, std::string& error)
{
    return set_value(KEY_LOCALIZATION_LANGUAGE, std::move(language), error);
}

int ConfigManager::window_width() const
{
    return get_typed_value_or<int>(KEY_WINDOW_WIDTH, 0);
}

bool ConfigManager::set_window_width(int value, std::string& error)
{
    return set_value(KEY_WINDOW_WIDTH, value, error);
}

int ConfigManager::window_height() const
{
    return get_typed_value_or<int>(KEY_WINDOW_HEIGHT, 0);
}

bool ConfigManager::set_window_height(int value, std::string& error)
{
    return set_value(KEY_WINDOW_HEIGHT, value, error);
}

bool ConfigManager::fullscreen() const
{
    return get_typed_value_or<bool>(KEY_WINDOW_FULLSCREEN, false);
}

bool ConfigManager::set_fullscreen(bool value, std::string& error)
{
    return set_value(KEY_WINDOW_FULLSCREEN, value, error);
}

double ConfigManager::target_fps() const
{
    return get_typed_value_or<double>(KEY_RENDER_FPS, 0.0);
}

bool ConfigManager::set_target_fps(double value, std::string& error)
{
    return set_value(KEY_RENDER_FPS, value, error);
}

bool ConfigManager::vsync() const
{
    return get_typed_value_or<bool>(KEY_RENDER_VSYNC, false);
}

bool ConfigManager::set_vsync(bool value, std::string& error)
{
    return set_value(KEY_RENDER_VSYNC, value, error);
}

int ConfigManager::master_volume() const
{
    return get_typed_value_or<int>(KEY_AUDIO_MASTER_VOLUME, 0);
}

bool ConfigManager::set_master_volume(int value, std::string& error)
{
    return set_value(KEY_AUDIO_MASTER_VOLUME, value, error);
}

int ConfigManager::music_volume() const
{
    return get_typed_value_or<int>(KEY_AUDIO_MUSIC_VOLUME, 0);
}

bool ConfigManager::set_music_volume(int value, std::string& error)
{
    return set_value(KEY_AUDIO_MUSIC_VOLUME, value, error);
}

int ConfigManager::sound_volume() const
{
    return get_typed_value_or<int>(KEY_AUDIO_SOUND_VOLUME, 0);
}

bool ConfigManager::set_sound_volume(int value, std::string& error)
{
    return set_value(KEY_AUDIO_SOUND_VOLUME, value, error);
}

void ConfigManager::register_runtime_settings(const elysia::bootstrap::RuntimeSettings& default_settings)
{
    _entries.clear();
    _values.clear();

    register_entry(
        KEY_WINDOW_WIDTH,
        ValueType::Int,
        read_window_width(default_settings),
        validate_positive_int,
        read_window_width,
        write_window_width);
    register_entry(
        KEY_WINDOW_HEIGHT,
        ValueType::Int,
        read_window_height(default_settings),
        validate_positive_int,
        read_window_height,
        write_window_height);
    register_entry(
        KEY_WINDOW_FULLSCREEN,
        ValueType::Bool,
        read_window_fullscreen(default_settings),
        validate_bool,
        read_window_fullscreen,
        write_window_fullscreen);
    register_entry(
        KEY_RENDER_FPS,
        ValueType::Double,
        read_render_fps(default_settings),
        validate_positive_double,
        read_render_fps,
        write_render_fps);
    register_entry(
        KEY_RENDER_VSYNC,
        ValueType::Bool,
        read_render_vsync(default_settings),
        validate_bool,
        read_render_vsync,
        write_render_vsync);
    register_entry(
        KEY_AUDIO_MASTER_VOLUME,
        ValueType::Int,
        read_audio_master_volume(default_settings),
        validate_volume,
        read_audio_master_volume,
        write_audio_master_volume);
    register_entry(
        KEY_AUDIO_MUSIC_VOLUME,
        ValueType::Int,
        read_audio_music_volume(default_settings),
        validate_volume,
        read_audio_music_volume,
        write_audio_music_volume);
    register_entry(
        KEY_AUDIO_SOUND_VOLUME,
        ValueType::Int,
        read_audio_sound_volume(default_settings),
        validate_volume,
        read_audio_sound_volume,
        write_audio_sound_volume);
    register_entry(
        KEY_LOCALIZATION_LANGUAGE,
        ValueType::String,
        read_localization_language(default_settings),
        validate_non_empty_string,
        read_localization_language,
        write_localization_language);
}

void ConfigManager::register_entry(
    std::string key,
    ValueType type,
    ConfigValue default_value,
    bool (*validator)(const ConfigValue& value, std::string& error),
    ConfigValue (*read_from_settings)(const elysia::bootstrap::RuntimeSettings& settings),
    void (*write_to_settings)(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
)
{
    _values[key] = default_value;
    _entries.emplace(
        std::move(key),
        ConfigEntry{
            type,
            std::move(default_value),
            validator,
            read_from_settings,
            write_to_settings
        });
}

void ConfigManager::load_values_from_settings(const elysia::bootstrap::RuntimeSettings& settings)
{
    for (const auto& [key, entry] : _entries)
    {
        if (!entry.read_from_settings)
            continue;

        _values[key] = entry.read_from_settings(settings);
    }
}

bool ConfigManager::matches_type(const ConfigValue& value, ValueType type)
{
    switch (type)
    {
    case ValueType::Bool:
        return std::holds_alternative<bool>(value);
    case ValueType::Int:
        return std::holds_alternative<int>(value);
    case ValueType::Double:
        return std::holds_alternative<double>(value);
    case ValueType::String:
        return std::holds_alternative<std::string>(value);
    default:
        return false;
    }
}
}
