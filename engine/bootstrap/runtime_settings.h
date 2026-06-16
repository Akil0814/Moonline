#pragma once

#include <string>

struct RuntimeSettings
{
    std::string window_title = "Moonline";
    int window_width = 1280;
    int window_height = 720;
    bool fullscreen = false;
    double target_fps = 60.0;
    bool vsync = true;
};

struct StartupParseResult
{
    bool success = false;
    RuntimeSettings runtime_settings;
    std::string error;
    bool rebuilt_user_config = false;

    explicit operator bool() const { return success; }
};
