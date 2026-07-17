#include "application.h"

#include "application_event_boundary.h"
#include "application_exit_policy.h"
#include "application_scene_composition.h"
#include "application_termination_logging.h"

#include "../audio/audio_service.h"
#include "../bootstrap/bootstrapper.h"
#include "../core/time.h"
#include "../loading/content_runtime_cleanup.h"
#include "../localization/localization_manager.h"
#include "../tools/logger.h"

#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <utility>

#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

namespace elysia::application
{
Application::~Application()
{
    shutdown();

    SDL_DestroyRenderer(_renderer);
    _renderer = nullptr;
    SDL_DestroyWindow(_window);
    _window = nullptr;

    Mix_CloseAudio();
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Application::init_assert(
    bool flag,
    const char* err_msg,
    std::source_location location)
{
    if (flag)
        return;

    const std::string error_message =
        err_msg ? err_msg : "Application runtime initialization failed.";
    elysia::tools::Logger::instance()->error("application",error_message,location);
    startup_fail(error_message,location);
}

[[noreturn]] void Application::startup_fail(
    const std::string& err_msg,
    std::source_location location)
{
    elysia::tools::Logger::instance()->terminating("application",err_msg,location);
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Game Start Error",
        err_msg.c_str(),
        _window);
    shutdown();
    std::exit(EXIT_FAILURE);
}

bool Application::init(
    int argc,
    char** argv,
    const IGameModule& game_module)
{
    _active = true;
    _normal_exit_requested = false;
    _has_shutdown = false;

    elysia::tools::TerminationManager::instance()->initialize_lifecycle();

    ApplicationDescriptor descriptor;
    try
    {
        descriptor = describe_game_module(game_module);
    }
    catch (const std::exception& error)
    {
        startup_fail(std::string("Game module descriptor failed: ") + error.what());
    }
    catch (...)
    {
        startup_fail("Game module descriptor failed with an unknown exception.");
    }

    if (descriptor.logical_width <= 0 || descriptor.logical_height <= 0)
        startup_fail("Game module logical viewport must be positive.");

    const std::filesystem::path executable_path =
        argc > 0 && argv && argv[0]
            ? std::filesystem::path(argv[0])
            : std::filesystem::path{};
    elysia::bootstrap::StartupParseResult parse_result =
        elysia::bootstrap::Bootstrapper::instance()->parse_runtime_settings(
            executable_path);

    if (!parse_result.success)
    {
        elysia::tools::Logger::instance()->error("bootstrap",parse_result.error);
        startup_fail(parse_result.error);
    }

    _content_registry = std::move(parse_result.content_registry);
    elysia::tools::Logger::instance()->initialize();

    if (!parse_result.warning.empty())
        ELYSIA_LOG_WARN("application","Startup warning: " << parse_result.warning);

    elysia::bootstrap::StartupSettings runtime_settings = parse_result.startup_settings;
    if (!init_runtime(runtime_settings,descriptor))
        startup_fail("Application runtime initialization failed.");

    if (!elysia::localization::LocalizationManager::instance()->init(
        _renderer,
        parse_result.i18n_manifest_path,
        runtime_settings.language))
    {
        startup_fail("Localization initialization failed.");
    }

    elysia::config::UserConfigService::instance()->register_user_config_change_handler(*this);
    _user_config_handler_registered = true;

    elysia::config::UserConfig& user_config =
        elysia::config::UserConfigService::instance()->user_config();
    if (user_config.language()
        != elysia::localization::LocalizationManager::instance()->current_language())
    {
        const auto language_result = user_config.set_language(
            elysia::localization::LocalizationManager::instance()->current_language());
        if (!language_result)
        {
            ELYSIA_LOG_WARN("application",
                "Localization warning: normalize language in config failed: "
                << language_result.error().message);
        }
        else if (const auto save_result =
            elysia::config::UserConfigService::instance()->save_user_config();
            !save_result)
        {
            ELYSIA_LOG_WARN("application",
                "Localization warning: save normalized language failed: "
                << save_result.error().message);
        }
    }

    _input_system.init();
    _input_system.set_renderer(_renderer);

    if (!elysia::bootstrap::Bootstrapper::instance()->preload_startup_resources(_renderer))
        startup_fail("Startup resource preload failed.");

    _scene_runtime_context.emplace(
        _renderer,
        _content_registry,
        descriptor.logical_width,
        descriptor.logical_height);
    _scene_manager.set_runtime_context(*_scene_runtime_context);

    enter_initial_scene(game_module,descriptor);
    return true;
}

bool Application::init_runtime(
    const elysia::bootstrap::StartupSettings& settings,
    const ApplicationDescriptor& descriptor)
{
    init_assert(!SDL_Init(SDL_INIT_EVERYTHING),"SDL2 Error");

    const int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    init_assert((IMG_Init(img_flags) & img_flags) == img_flags,"SDL_image Error");

    const int mix_flags = MIX_INIT_MP3;
    init_assert((Mix_Init(mix_flags) & mix_flags) == mix_flags,"SDL_mixer Error");

    init_assert(!TTF_Init(),"SDL_ttf Error");
    init_assert(
        Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048) == 0,
        "Mix_OpenAudio Error");
    init_assert(
        elysia::audio::AudioService::instance()->init(settings.audio),
        "AudioService init failed");

    SDL_SetHint(SDL_HINT_IME_SHOW_UI,"1");

    _window = SDL_CreateWindow(
        settings.window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        settings.window_width,
        settings.window_height,
        SDL_WINDOW_SHOWN);
    init_assert(_window,"SDL_CreateWindow Error");

    if (settings.fullscreen
        && SDL_SetWindowFullscreen(_window,SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
    {
        ELYSIA_LOG_WARN("application","Failed to enter fullscreen: " << SDL_GetError());
        SDL_ClearError();
        SDL_SetWindowSize(_window,settings.window_width,settings.window_height);
        SDL_SetWindowPosition(_window,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);
    }

    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    if (settings.vsync)
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;

    _renderer = SDL_CreateRenderer(_window,-1,renderer_flags);
    init_assert(_renderer,"SDL_CreateRenderer Error");
    init_assert(
        SDL_RenderSetLogicalSize(
            _renderer,
            descriptor.logical_width,
            descriptor.logical_height) == 0,
        "SDL_RenderSetLogicalSize Error");

    _target_fps = settings.target_fps;
    return true;
}

void Application::enter_initial_scene(
    const IGameModule& game_module,
    const ApplicationDescriptor& descriptor)
{
    _scene_manager.attach(this);

    try
    {
        compose_application_scenes(_scene_manager,game_module,descriptor);
    }
    catch (const std::exception& error)
    {
        startup_fail(std::string("Scene composition failed: ") + error.what());
    }
    catch (...)
    {
        startup_fail("Scene composition failed with an unknown exception.");
    }
}

int Application::run()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Uint64 last_counter = SDL_GetPerformanceCounter();
    const Uint64 counter_freq = SDL_GetPerformanceFrequency();
    elysia::core::Time::instance()->reset();

    int exit_code = EXIT_SUCCESS;
    auto resolve_exit = [this,&exit_code]()
    {
        auto* termination_manager = elysia::tools::TerminationManager::instance();
        const ApplicationExitDecision decision =
            resolve_application_exit(_normal_exit_requested,*termination_manager);
        if (decision == ApplicationExitDecision::Continue)
            return false;

        _active = false;
        if (decision == ApplicationExitDecision::FaultExit)
            exit_code = EXIT_FAILURE;
        log_fault_exit_if_needed(decision,termination_manager->termination_info());
        return true;
    };

    while (_active)
    {
        _input_system.begin_frame();
        while (SDL_PollEvent(&_event))
        {
            _input_system.process_event(_event);
            if (_event.type == SDL_QUIT)
                _normal_exit_requested = true;
        }

        _input_system.end_frame();
        if (resolve_exit())
            break;

        if (!run_event_boundary("input",[this]()
        {
            _scene_manager.on_input(_input_system.frame(),_input_system.events());
        }))
        {
            if (!resolve_exit())
            {
                _active = false;
                exit_code = EXIT_FAILURE;
            }
            break;
        }
        if (resolve_exit())
            break;

        const Uint64 current_counter = SDL_GetPerformanceCounter();
        const double delta =
            static_cast<double>(current_counter - last_counter) / counter_freq;
        last_counter = current_counter;
        elysia::core::Time::instance()->begin_frame(delta);

        if (delta * 1000.0 < 1000.0 / _target_fps)
        {
            SDL_Delay(static_cast<Uint32>(
                1000.0 / _target_fps - delta * 1000.0));
        }

        if (!run_event_boundary("update",[this]()
        {
            const double frame_delta = elysia::core::Time::instance()->delta();
            _scene_manager.on_update(frame_delta);
            elysia::audio::AudioService::instance()->update(frame_delta);
        }))
        {
            if (!resolve_exit())
            {
                _active = false;
                exit_code = EXIT_FAILURE;
            }
            break;
        }
        if (resolve_exit())
            break;

        SDL_SetRenderDrawColor(_renderer,0,0,0,255);
        SDL_RenderClear(_renderer);

        if (!run_event_boundary("render",[this]()
        {
            _scene_manager.on_render(_renderer);
        }))
        {
            if (!resolve_exit())
            {
                _active = false;
                exit_code = EXIT_FAILURE;
            }
            break;
        }
        if (resolve_exit())
            break;

        SDL_RenderPresent(_renderer);
        if (resolve_exit())
            break;
    }

    shutdown();
    return exit_code;
}

void Application::shutdown()
{
    if (_has_shutdown)
        return;

    _has_shutdown = true;

    _input_system.shutdown();
    _scene_manager.detach(this);
    _scene_manager.shutdown();
    _scene_runtime_context.reset();
    elysia::bootstrap::Bootstrapper::instance()->release_preload_textures();

    elysia::localization::LocalizationManager::instance()->shutdown();
    if (_user_config_handler_registered)
    {
        elysia::config::UserConfigService::instance()
            ->unregister_user_config_change_handler(*this);
        _user_config_handler_registered = false;
    }
    elysia::config::UserConfigService::instance()->shutdown();
    elysia::audio::AudioService::instance()->shutdown();
    elysia::loading::clear_loaded_content();
    ELYSIA_LOG("application","Application shutdown complete");
    elysia::tools::Logger::instance()->shutdown();
}

void Application::on_scene_manager_quit_requested()
{
    _normal_exit_requested = true;
}

namespace
{
std::unexpected<elysia::config::UserConfigFailure> runtime_apply_failure(
    const char* setting,
    const std::string& message)
{
    return std::unexpected(elysia::config::UserConfigFailure{
        elysia::config::UserConfigError::RuntimeApplyFailed,
        setting,
        message
    });
}
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_master_volume(int value)
{
    elysia::audio::AudioService::instance()->set_master_volume(value);
    return {};
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_music_volume(int value)
{
    elysia::audio::AudioService::instance()->set_music_volume(value);
    return {};
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_sound_volume(int value)
{
    elysia::audio::AudioService::instance()->set_sound_volume(value);
    return {};
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_language(std::string_view language)
{
    if (!elysia::localization::LocalizationManager::instance()->set_language(
        std::string(language)))
    {
        return runtime_apply_failure("language","Runtime language change failed.");
    }

    elysia::localization::LocalizationManager::instance()->clear_texture_cache();
    return {};
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_target_fps(double value)
{
    if (value <= 0.0)
        return runtime_apply_failure("target_fps","Target FPS must be positive.");
    _target_fps = value;
    return {};
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_window_size(int width,int height)
{
    if (!_window)
        return runtime_apply_failure("window_size","Application window is unavailable.");
    SDL_SetWindowSize(_window,width,height);
    return {};
}

std::expected<void,elysia::config::UserConfigFailure>
Application::apply_fullscreen(bool value)
{
    if (!_window)
        return runtime_apply_failure("fullscreen","Application window is unavailable.");

    const Uint32 flags = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    if (SDL_SetWindowFullscreen(_window,flags) != 0)
        return runtime_apply_failure("fullscreen",SDL_GetError());
    return {};
}
}
