#pragma once

#include "../bootstrap/runtime_settings.h"
#include "../bootstrap/user_config_store.h"
#include "../tools/singleton.h"
#include "config_value.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace elysia::config
{
struct ConfigInitResult
{
    bool success = false;
    elysia::bootstrap::RuntimeSettings runtime_settings;
    std::string error;
    std::string warning;
    bool rebuilt_user_config = false;
};

class ConfigManager : public elysia::tools::Singleton<ConfigManager>
{
    friend elysia::tools::Singleton<ConfigManager>;

public:
    ConfigInitResult init(
        const elysia::bootstrap::RuntimeSettings& default_settings,
        const std::filesystem::path& user_config_path
    );
    void shutdown();

    bool save(std::string& error);
    bool is_dirty() const;

    bool has_value(std::string_view key) const;
    const ConfigValue* get_value(std::string_view key) const;
    bool set_value(std::string_view key, const ConfigValue& value, std::string& error);

    elysia::bootstrap::RuntimeSettings snapshot_runtime_settings() const;

    std::string_view language() const;
    bool set_language(std::string language, std::string& error);

    int window_width() const;
    bool set_window_width(int value, std::string& error);

    int window_height() const;
    bool set_window_height(int value, std::string& error);

    bool fullscreen() const;
    bool set_fullscreen(bool value, std::string& error);

    double target_fps() const;
    bool set_target_fps(double value, std::string& error);

    bool vsync() const;
    bool set_vsync(bool value, std::string& error);

    int master_volume() const;
    bool set_master_volume(int value, std::string& error);

    int music_volume() const;
    bool set_music_volume(int value, std::string& error);

    int sound_volume() const;
    bool set_sound_volume(int value, std::string& error);

private:
    enum class ValueType
    {
        Bool,
        Int,
        Double,
        String
    };

    struct ConfigEntry
    {
        ValueType type;
        ConfigValue default_value;
        bool (*validator)(const ConfigValue& value, std::string& error) = nullptr;
        ConfigValue (*read_from_settings)(const elysia::bootstrap::RuntimeSettings& settings) = nullptr;
        void (*write_to_settings)(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings) = nullptr;
    };

    void register_runtime_settings(const elysia::bootstrap::RuntimeSettings& default_settings);
    void register_entry(
        std::string key,
        ValueType type,
        ConfigValue default_value,
        bool (*validator)(const ConfigValue& value, std::string& error),
        ConfigValue (*read_from_settings)(const elysia::bootstrap::RuntimeSettings& settings),
        void (*write_to_settings)(const ConfigValue& value, elysia::bootstrap::RuntimeSettings& settings)
    );
    void load_values_from_settings(const elysia::bootstrap::RuntimeSettings& settings);
    static bool matches_type(const ConfigValue& value, ValueType type);

    template <typename T>
    T get_typed_value_or(std::string_view key, const T& fallback) const
    {
        const ConfigValue* value = get_value(key);
        if (!value)
            return fallback;
        return std::get<T>(*value);
    }

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    elysia::bootstrap::UserConfigStore _user_config_store;
    std::filesystem::path _user_config_path;
    elysia::bootstrap::RuntimeSettings _base_runtime_settings;
    std::unordered_map<std::string, ConfigEntry> _entries;
    std::unordered_map<std::string, ConfigValue> _values;
    bool _dirty = false;
    bool _initialized = false;
};
}
