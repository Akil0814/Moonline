#pragma once

#include "game_module.h"

#include "../bootstrap/runtime_settings.h"
#include "../config/user_config_service.h"
#include "../input/input_system.h"
#include "../io/loaders/asset_config_types.h"
#include "../scene/scene_manager.h"
#include "../scene/scene_manager_observer.h"
#include "../scene/scene_runtime_context.h"
#include "../tools/singleton.h"

#include <SDL.h>

#include <expected>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace elysia::application
{
class Application final
    : public elysia::tools::Singleton<Application>
    , public elysia::scene::SceneManagerObserver
    , public elysia::config::IUserConfigChangeHandler
{
    friend elysia::tools::Singleton<Application>;

public:
    ~Application();

    bool init(int argc,char** argv,const IGameModule& game_module);
    int run();

    std::expected<void,elysia::config::UserConfigFailure> apply_master_volume(int value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_music_volume(int value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_sound_volume(int value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_language(std::string_view language) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_target_fps(double value) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_window_size(int width,int height) override;
    std::expected<void,elysia::config::UserConfigFailure> apply_fullscreen(bool value) override;

private:
    Application() = default;

    bool init_runtime(
        const elysia::bootstrap::StartupSettings& settings,
        const ApplicationDescriptor& descriptor);
    void enter_initial_scene(
        const IGameModule& game_module,
        const ApplicationDescriptor& descriptor);
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
    double _target_fps = 60.0;

    SDL_Event _event{};
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;

    elysia::input::InputSystem _input_system;
    elysia::scene::SceneManager _scene_manager;
    elysia::io::ContentRegistry _content_registry;
    std::optional<elysia::scene::SceneRuntimeContext> _scene_runtime_context;

    bool _active = true;
    bool _normal_exit_requested = false;
    bool _has_shutdown = false;
    bool _user_config_handler_registered = false;
};
}
