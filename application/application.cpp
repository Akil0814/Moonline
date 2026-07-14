#include "application.h"
#include "application_event_boundary.h"
#include "application_exit_policy.h"
#include "application_termination_logging.h"
#include "scene/scene_keys.h"
#include "scene/scene_registry.h"

#include "../engine/audio/audio_service.h"
#include "../engine/bootstrap/bootstrapper.h"
#include "../engine/config/config_service.h"
#include "../engine/core/time.h"
#include "../engine/localization/localization_manager.h"
#include "../engine/resources/resource_manager.h"
#include "../engine/tools/logger.h"

#include <cstdlib>
#include <ctime>
#include <expected>

#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>


Application::Application() = default;

Application:: ~Application()
{
	shutdown();
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);

	Mix_CloseAudio();
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Application::init_assert(bool flag,const char* err_msg,std::source_location location)
{
	if (flag)
		return;
	const std::string error_message = err_msg ? err_msg : "Application runtime initialization failed.";
	elysia::tools::Logger::instance()->error("application",error_message,location);
	startup_fail(error_message,location);
}

[[noreturn]] void Application::startup_fail(const std::string& err_msg,std::source_location location)
{
	elysia::tools::Logger::instance()->terminating("application",err_msg,location);
	SDL_ShowSimpleMessageBox(
		SDL_MESSAGEBOX_ERROR,
		"Game Start Error",
		err_msg.c_str(),
		_window);
	std::exit(EXIT_FAILURE);
}

bool Application::init(int argc, char** argv)
{
	elysia::tools::TerminationManager::instance()->initialize_lifecycle();

	const elysia::bootstrap::StartupParseResult parse_result =
		elysia::bootstrap::Bootstrapper::instance()->parse_runtime_settings();

	if (!parse_result.success)
	{
		elysia::tools::Logger::instance()->error("bootstrap",parse_result.error);
		startup_fail(parse_result.error);
		return false;
	}

	elysia::tools::Logger::instance()->initialize();

	if (!parse_result.warning.empty())
	{
		ELYSIA_LOG_WARN("application","Startup warning: " << parse_result.warning);
	}

	elysia::bootstrap::RuntimeSettings runtime_settings = parse_result.runtime_settings;

	if (!init_runtime(runtime_settings))
	{
		elysia::tools::Logger::instance()->error("application","Application runtime initialization failed.");
		startup_fail("Application runtime initialization failed.");
		return false;
	}

	if (!elysia::localization::LocalizationManager::instance()->init(
		_renderer,
		parse_result.i18n_manifest_path,
		runtime_settings.language))
	{
		elysia::tools::Logger::instance()->error("localization","Localization initialization failed.");
		startup_fail("Localization initialization failed.");
		return false;
	}

	elysia::config::ConfigService::instance()->register_settings_change_handler(*this);
	_settings_handler_registered = true;

	elysia::config::UserSettings& user_settings = elysia::config::ConfigService::instance()->user_settings();
	if (user_settings.language() != elysia::localization::LocalizationManager::instance()->current_language())
	{
		const auto language_result = user_settings.set_language(
			elysia::localization::LocalizationManager::instance()->current_language());
		if (!language_result)
		{
			ELYSIA_LOG_WARN("application","Localization warning: normalize language in config failed: "
				<< language_result.error().message);
		}
		else if (const auto save_result = elysia::config::ConfigService::instance()->save_user_settings(); !save_result)
		{
			ELYSIA_LOG_WARN("application","Localization warning: save normalized language failed: "
				<< save_result.error().message);
		}
	}

	_input_system.init();
	_input_system.set_renderer(_renderer);

	if (!elysia::bootstrap::Bootstrapper::instance()->preload_startup_resources(_renderer))
	{
		elysia::tools::Logger::instance()->error("bootstrap","Startup resource preload failed.");
		startup_fail("Startup resource preload failed.");
		return false;
	}

	enter_startup_scene();

	return true;
}

bool Application::init_runtime(const elysia::bootstrap::RuntimeSettings& settings)
{
	init_assert(!SDL_Init(SDL_INIT_EVERYTHING), "SDL2 Error");

	const int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
	init_assert((IMG_Init(img_flags) & img_flags) == img_flags, "SDL_image Error");

	const int mix_flags = MIX_INIT_MP3;
	init_assert((Mix_Init(mix_flags) & mix_flags) == mix_flags, "SDL_mixer Error");

	init_assert(!TTF_Init(), "SDL_ttf Error");
	init_assert(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0, "Mix_OpenAudio Error");
	init_assert(elysia::audio::AudioService::instance()->init(settings.audio), "AudioService init failed");

	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	_window = SDL_CreateWindow(
		settings.window_title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		settings.window_width,
		settings.window_height,
		SDL_WINDOW_SHOWN);
	init_assert(_window, "SDL_CreateWindow Error");

	if (settings.fullscreen
		&& SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
	{
		elysia::tools::Logger::instance()->warn("application","Failed to enter fullscreen");
		elysia::tools::Logger::instance()->warn("application",SDL_GetError());
		SDL_ClearError();

		SDL_SetWindowSize(_window, settings.window_width, settings.window_height);
		SDL_SetWindowPosition(_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	if (settings.vsync)
		renderer_flags |= SDL_RENDERER_PRESENTVSYNC;

	_renderer = SDL_CreateRenderer(_window, -1, renderer_flags);
	init_assert(_renderer, "SDL_CreateRenderer Error");

	init_assert(
		SDL_RenderSetLogicalSize(_renderer, _logical_width, _logical_height) == 0,
		"SDL_RenderSetLogicalSize Error");

	FPS = settings.target_fps;
	return true;
}

void Application::enter_startup_scene()
{
	_scene_manager.attach(this);

	register_all_scenes(_scene_manager);

	_scene_manager.start(AppSceneKeys::StartupLoading);
}

int  Application::run(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	std::srand(static_cast<unsigned>(std::time(nullptr)));

	Uint64 last_counter = SDL_GetPerformanceCounter();
	const Uint64 counter_freq = SDL_GetPerformanceFrequency();

	elysia::core::Time::instance()->reset();


	_counter_freq = SDL_GetPerformanceFrequency();
	_last_counter = SDL_GetPerformanceCounter();
	int exit_code = EXIT_SUCCESS;
	auto resolve_exit = [this,&exit_code]()
	{
		auto* termination_manager = elysia::tools::TerminationManager::instance();
		const moonline::application::ApplicationExitDecision decision =
			moonline::application::resolve_application_exit(_normal_exit_requested,*termination_manager);
		if (decision == moonline::application::ApplicationExitDecision::Continue)
			return false;

		_active = false;
		if (decision == moonline::application::ApplicationExitDecision::FaultExit)
			exit_code = EXIT_FAILURE;
		moonline::application::log_fault_exit_if_needed(
			decision,termination_manager->termination_info());
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

		if (!moonline::application::run_event_boundary("input",[this]()
		{
			_scene_manager.on_input(_input_system.frame(), _input_system.events());
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

		Uint64 current_counter = SDL_GetPerformanceCounter();
		double delta = (double)(current_counter - last_counter) / counter_freq;
		last_counter = current_counter;
		elysia::core::Time::instance()->begin_frame(delta);

		if (delta * 1000 < 1000.0 / FPS)
			SDL_Delay((Uint32)(1000.0 / FPS - delta * 1000));
		

		if (!moonline::application::run_event_boundary("update",[this]()
		{
			_scene_manager.on_update(elysia::core::Time::instance()->delta());
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

		SDL_SetRenderDrawColor(_renderer, 0,0,0,255);
		SDL_RenderClear(_renderer);

		if (!moonline::application::run_event_boundary("render",[this]()
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
	elysia::localization::LocalizationManager::instance()->shutdown();
	if (_settings_handler_registered)
	{
		elysia::config::ConfigService::instance()->unregister_settings_change_handler(*this);
		_settings_handler_registered = false;
	}
	elysia::config::ConfigService::instance()->shutdown();
	elysia::audio::AudioService::instance()->shutdown();
	elysia::resources::ResourceManager::instance()->clear();
	elysia::tools::Logger::instance()->info("application","Application shutdown complete");
	elysia::tools::Logger::instance()->shutdown();
}

void Application::on_scene_manager_quit_requested()
{
	_normal_exit_requested = true;
}

namespace
{
std::unexpected<elysia::config::UserSettingsFailure> runtime_apply_failure(
	const char* setting,const std::string& message)
{
	return std::unexpected(elysia::config::UserSettingsFailure{
		elysia::config::UserSettingsError::RuntimeApplyFailed,setting,message});
}
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_master_volume(int value)
{
	elysia::audio::AudioService::instance()->set_master_volume(value);
	return {};
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_music_volume(int value)
{
	elysia::audio::AudioService::instance()->set_music_volume(value);
	return {};
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_sound_volume(int value)
{
	elysia::audio::AudioService::instance()->set_sound_volume(value);
	return {};
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_language(std::string_view language)
{
	if (!elysia::localization::LocalizationManager::instance()->set_language(std::string(language)))
		return runtime_apply_failure("language","Runtime language change failed.");
	elysia::localization::LocalizationManager::instance()->clear_texture_cache();
	return {};
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_target_fps(double value)
{
	if (value <= 0.0) return runtime_apply_failure("target_fps","Target FPS must be positive.");
	FPS = value;
	return {};
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_window_size(int width,int height)
{
	if (!_window) return runtime_apply_failure("window_size","Application window is unavailable.");
	SDL_SetWindowSize(_window,width,height);
	return {};
}

std::expected<void,elysia::config::UserSettingsFailure> Application::apply_fullscreen(bool value)
{
	if (!_window) return runtime_apply_failure("fullscreen","Application window is unavailable.");
	const Uint32 flags = value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
	if (SDL_SetWindowFullscreen(_window,flags) != 0)
		return runtime_apply_failure("fullscreen",SDL_GetError());
	return {};
}
