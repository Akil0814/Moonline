#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "config_value.h"
#include "../tools/singleton.h"

class ConfigManager: public Singleton<ConfigManager>
{
    friend Singleton<ConfigManager>;
public:
    ConfigManager() = default;
    ~ConfigManager() = default;

    bool load_default_config(const std::filesystem::path& path);
    bool load_user_config(const std::filesystem::path& path);
    bool save_user_config(const std::filesystem::path& path) const;

    [[nodiscard]] bool has(std::string_view key) const;

    [[nodiscard]] bool get_bool(std::string_view key, bool default_value = false) const;
    [[nodiscard]] int get_int(std::string_view key, int default_value = 0) const;
    [[nodiscard]] float get_float(std::string_view key, float default_value = 0.0f) const;
    [[nodiscard]] std::string get_string(
        std::string_view key,
        std::string_view default_value = ""
    ) const;

    void set_bool(std::string_view key, bool value);
    void set_int(std::string_view key, int value);
    void set_float(std::string_view key, float value);
    void set_string(std::string_view key, std::string value);

private:
    std::unordered_map<std::string, ConfigValue> _values;
};