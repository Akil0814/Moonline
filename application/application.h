#pragma once
#include "../engine/scene/scene_manager.h"
#include "../engine/scene/scene_manager_observer.h"
#include "../engine/tools/singleton.h"
#include "../engine/input/input_system.h"
#include "../engine/bootstrap/runtime_settings.h"
#include "../engine/config/user_config_service.h"

#include <SDL.h>

#include <expected>
#include <source_location>
#include <string>
#include <string_view>

class Application
    : public elysia::tools::Singleton<Application>
    , public elysia::scene::SceneManagerObserver
    , public elysia::config::IUserConfigChangeHandler
{
    friend elysia::tools::Singleton<Application>;
public:
    Application();
    ~Application();

    bool init(int argc, char** argv);
    int run(int argc, char** argv);
    SDL_Renderer* renderer() const { return _renderer; }
    const elysia::io::ContentRegistry& content_registry() const noexcept { return _content_registry; }

    std::expected<void,elysia::config::UserConfigFailure> apply_master_volume(int value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_music_volume(int value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_sound_volume(int value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_language(std::string_view language) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_target_fps(double value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_window_size(int width,int height) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_fullscreen(bool value) override;

private:
    bool init_runtime(const elysia::bootstrap::StartupSettings& settings);
    void enter_startup_scene();

    void shutdown();

    void on_scene_manager_quit_requested() override;


    void init_assert(
        bool flag,
        const char* err_msg,
        std::source_location location = std::source_location::current());
    [[noreturn]] void startup_fail(
        const std::string& err_msg,
        std::source_location location = std::source_location::current());

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
    elysia::io::ContentRegistry _content_registry;

    bool _active = { true };
    bool _normal_exit_requested = false;
    bool _has_shutdown = false;
    bool _user_config_handler_registered = false;

};
