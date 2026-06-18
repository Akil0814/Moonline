#pragma once

#include "runtime_settings.h"
#include "../io/json/json_loader.h"

#include <filesystem>
#include <string>

class UserConfigStore
{
public:
    struct Result
    {
        bool success = false;
        RuntimeSettings runtime_settings;
        std::string error;
        std::string warning;
        bool rebuilt_user_config = false;

        explicit operator bool() const { return success; }
    };

    Result load_or_create(
        const std::filesystem::path& user_config_path,
        const RuntimeSettings& default_settings
    ) const;
    bool save(
        const std::filesystem::path& user_config_path,
        const RuntimeSettings& runtime_settings,
        std::string& error
    ) const;

private:
    static bool read_positive_int_override(
        const json& node,
        const char* key,
        int& out,
        std::string& error
    );
    static bool read_positive_double_override(
        const json& node,
        const char* key,
        double& out,
        std::string& error
    );
    static bool read_volume_override(
        const json& node,
        const char* key,
        int& out,
        std::string& error
    );
    static bool read_bool_override(
        const json& node,
        const char* key,
        bool& out,
        std::string& error
    );
    static json make_user_config_json(const RuntimeSettings& runtime_settings);

    bool apply_overrides(
        const std::filesystem::path& user_config_path,
        RuntimeSettings& runtime_settings,
        std::string& error
    ) const;
    bool write_user_config(
        const std::filesystem::path& user_config_path,
        const RuntimeSettings& runtime_settings,
        std::string& error
    ) const;
};
