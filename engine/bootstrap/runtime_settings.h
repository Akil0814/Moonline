#pragma once

#include "../audio/audio_settings.h"

#include <filesystem>
#include <string>

struct RuntimeSettings
{
    std::string window_title = "Moonline";
    int window_width = 1280;
    int window_height = 720;
    bool fullscreen = false;
    double target_fps = 60.0;
    bool vsync = true;
    std::string language;
    AudioSettings audio;
};

struct StartupParseResult
{
    bool success = false;
    RuntimeSettings runtime_settings;
    std::filesystem::path i18n_manifest_path;
    std::string error;
    std::string warning;
    bool rebuilt_user_config = false;

    explicit operator bool() const { return success; }
};
