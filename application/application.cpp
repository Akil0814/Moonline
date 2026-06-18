#include "application.h"
#include "scene/scene_keys.h"
#include "scene/scene_registry.h"

#include "../engine/audio/audio_service.h"
#include "../engine/bootstrap/bootstrapper.h"
#include "../engine/core/time.h"
#include "../engine/resources/resource_manager.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

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

bool Application::init(int argc, char** argv)
{
	const StartupParseResult parse_result =
		Bootstrapper::instance()->parse_runtime_settings();

	if (!parse_result.success)
	{
		startup_fail(parse_result.error);
		return false;
	}

	if (!parse_result.warning.empty())
	{
		std::cout << "Startup warning: " << parse_result.warning << std::endl;
	}

	if (!init_runtime(parse_result.runtime_settings))
	{
		startup_fail("Application runtime initialization failed.");
		return false;
	}

	_input_system.init();
	_input_system.set_renderer(_renderer);

	if (!Bootstrapper::instance()->preload_startup_resources(_renderer))
	{
		startup_fail("Startup resource preload failed.");
		return false;
	}

	enter_startup_scene();

	return true;
}

bool Application::init_runtime(const RuntimeSettings& settings)
{
	init_assert(!SDL_Init(SDL_INIT_EVERYTHING), "SDL2 Error");

	const int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
	init_assert((IMG_Init(img_flags) & img_flags) == img_flags, "SDL_image Error");

	const int mix_flags = MIX_INIT_MP3;
	init_assert((Mix_Init(mix_flags) & mix_flags) == mix_flags, "SDL_mixer Error");

	init_assert(!TTF_Init(), "SDL_ttf Error");
	init_assert(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0, "Mix_OpenAudio Error");
	init_assert(AudioService::instance()->init(settings.audio), "AudioService init failed");

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
		SDL_Log("Failed to enter fullscreen: %s", SDL_GetError());
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

	Time::instance()->reset();


	_counter_freq = SDL_GetPerformanceFrequency();
	_last_counter = SDL_GetPerformanceCounter();


	while (_active)
	{
		_input_system.begin_frame();
		while (SDL_PollEvent(&_event))
		{
			if (_event.type == SDL_QUIT)
				_active = false;
			_input_system.process_event(_event);
		}

		_input_system.end_frame();

		_scene_manager.on_input(
			_input_system.frame(),
			_input_system.events()
		);

		Uint64 current_counter = SDL_GetPerformanceCounter();//实现动态延时
		double delta = (double)(current_counter - last_counter) / counter_freq;
		last_counter = current_counter;
		Time::instance()->begin_frame(delta);

		if (delta * 1000 < 1000.0 / FPS)
			SDL_Delay((Uint32)(1000.0 / FPS - delta * 1000));
		

		_scene_manager.on_update(Time::instance()->delta());

		SDL_SetRenderDrawColor(_renderer, 0,0,0,255);
		SDL_RenderClear(_renderer);

		_scene_manager.on_render(_renderer);

		SDL_RenderPresent(_renderer);
	}

	shutdown();

    return 0;
}

void Application::shutdown()
{
    if (_has_shutdown)
        return;

	_has_shutdown = true;

	_input_system.shutdown();
	_scene_manager.detach(this);
	_scene_manager.shutdown();
	AudioService::instance()->shutdown();
	ResourceManager::instance()->clear();
}

void Application::on_scene_manager_quit_requested()
{
	_active = false;
}
