#pragma once

#include "../audio/audio_settings.h"
#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <string>

namespace elysia::bootstrap
{
enum class WindowMode
{
    Windowed,
    BorderlessFullscreen
};

struct WindowSize
{
    int width = 1280;
    int height = 720;

    friend bool operator==(const WindowSize&,const WindowSize&) = default;
};

struct WindowSettings
{
    WindowMode mode = WindowMode::Windowed;
    WindowSize windowed_size{};

    friend bool operator==(const WindowSettings&,const WindowSettings&) = default;
};

struct UserConfigData
{
    WindowSettings window;
    double target_fps = 60.0;
    bool vsync = true;
    std::string language;
    elysia::audio::AudioSettings audio;

    friend bool operator==(const UserConfigData&,const UserConfigData&) = default;
};

struct AppConfig
{
    std::string window_title = "Moonline";
    UserConfigData user_defaults;
};

struct StartupSettings : UserConfigData
{
    std::string window_title = "Moonline";

    friend bool operator==(const StartupSettings&,const StartupSettings&) = default;
};

struct StartupParseResult
{
    bool success = false;
    StartupSettings startup_settings;
    elysia::io::ContentRegistry content_registry;
    std::filesystem::path i18n_manifest_path;
    std::string error;
    std::string warning;
    bool recovered_user_config = false;
    bool rebuilt_user_config = false;

    explicit operator bool() const { return success; }
};

}
