#pragma once
#include "../engine/scene/scene_manager.h"
#include "../engine/scene/scene_manager_observer.h"
#include "../engine/tools/singleton.h"
#include "../engine/input/input_system.h"
#include "../engine/bootstrap/runtime_settings.h"
#include "../engine/config/i_settings_change_handler.h"

#include <SDL.h>

#include <expected>
#include <string>
#include <string_view>

class Application
    : public elysia::tools::Singleton<Application>
    , public elysia::scene::SceneManagerObserver
    , public elysia::config::ISettingsChangeHandler
{
    friend elysia::tools::Singleton<Application>;
public:
    Application();
    ~Application();

    bool init(int argc, char** argv);
    int run(int argc, char** argv);
    SDL_Renderer* renderer() const { return _renderer; }

    std::expected<void,elysia::config::UserSettingsFailure> apply_master_volume(int value) override;
    std::expected<void,elysia::config::UserSettingsFailure> apply_music_volume(int value) override;
    std::expected<void,elysia::config::UserSettingsFailure> apply_sound_volume(int value) override;
    std::expected<void,elysia::config::UserSettingsFailure> apply_language(std::string_view language) override;
    std::expected<void,elysia::config::UserSettingsFailure> apply_target_fps(double value) override;
    std::expected<void,elysia::config::UserSettingsFailure> apply_window_size(int width,int height) override;
    std::expected<void,elysia::config::UserSettingsFailure> apply_fullscreen(bool value) override;

private:
    bool init_runtime(const elysia::bootstrap::RuntimeSettings& settings);
    void enter_startup_scene();

    void shutdown();

    void on_scene_manager_quit_requested() override;


    void init_assert(bool flag, const char* err_msg)
    {
        if (flag)
            return;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game Start Error", err_msg, _window);
        exit(-1);
    }
    void startup_fail(const std::string& err_msg)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Game Start Error",
            err_msg.c_str(),
            _window);
        exit(-1);
    }

private:
    const int _logical_width = 1280;
    const int _logical_height = 720;

    double FPS = 60;

    Uint64 _last_counter = 0;
    Uint64 _counter_freq = 0;

    SDL_Event _event;
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;

    elysia::input::InputSystem _input_system;
    elysia::scene::SceneManager _scene_manager;

    bool _active = { true };
    bool _normal_exit_requested = false;
    bool _has_shutdown = false;
    bool _settings_handler_registered = false;

};
