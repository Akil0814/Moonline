#include "application.h"
#include "scene/scene_keys.h"
#include "scene/scene_registry.h"

#include "../engine/core/time.h"

#include "../engine/resources/resource_manager.h"
#include "../engine/resources/resource_bootstrapper.h"

#include "../gameplay/scene/loading_scene.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <filesystem>
#include <algorithm>

#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>


Application:: Application()
{
	init_assert(!SDL_Init(SDL_INIT_EVERYTHING), "SDL2 Error");

	int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
	init_assert((IMG_Init(img_flags) & img_flags) == img_flags, "SDL_image Error");

	int mix_flags = MIX_INIT_MP3;
	init_assert((Mix_Init(mix_flags) & mix_flags) == mix_flags, "SDL_mixer Error");

	init_assert(!TTF_Init(), "SDL_ttf Error");
	init_assert(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0, "Mix_OpenAudio Error");

	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	_window = SDL_CreateWindow("Moonline", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _logical_width, _logical_height, SDL_WINDOW_SHOWN);
	init_assert(_window, "SDL_CreateWindow Error");

	if (SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
	{
		SDL_Log("Failed to enter fullscreen: %s", SDL_GetError());
		SDL_ClearError();

		//fallback
		SDL_SetWindowSize(_window, _logical_width, _logical_height);
		SDL_SetWindowPosition(_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);//硬件加速 垂直同步 目标纹理
	init_assert(_renderer, "SDL_CreateRenderer Error");

	init_assert(SDL_RenderSetLogicalSize(_renderer, _logical_width, _logical_height) == 0, "SDL_RenderSetLogicalSize Error");
	open_connected_controllers();
}

Application:: ~Application()
{
	close_all_controllers();
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);

	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool Application::init(int argc, char** argv)
{  
	_input_system.set_context(InputContext::UI);

	ResourceBootstrapper::instance()->bootstrap(_renderer);

	_scene_manager.attach(this);

	register_all_scenes(_scene_manager);

	_scene_manager.start(AppSceneKeys::StartupLoading);

    return true;
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
			handle_controller_device_event(_event);
			_input_system.process_event(_event);
		}

		_input_system.end_frame();

		_scene_manager.on_input(
			_input_system.snapshot(),
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
    {
        return;
    }

	_has_shutdown = true;

	close_all_controllers();
	_scene_manager.detach(this);
	_scene_manager.shutdown();
	ResourceManager::instance()->clear();
}

void Application::on_scene_manager_quit_requested()
{
	_active = false;
}

void Application::open_connected_controllers()
{
	const int joystick_count = SDL_NumJoysticks();
	for (int joystick_index = 0; joystick_index < joystick_count; ++joystick_index)
	{
		open_controller(joystick_index);
	}
}

void Application::open_controller(int joystick_index)
{
	if (!SDL_IsGameController(joystick_index))
	{
		return;
	}

	SDL_GameController* controller = SDL_GameControllerOpen(joystick_index);
	if (!controller)
	{
		SDL_Log("Failed to open controller %d: %s", joystick_index, SDL_GetError());
		SDL_ClearError();
		return;
	}

	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
	const SDL_JoystickID joystick_id = SDL_JoystickInstanceID(joystick);

	for (SDL_GameController* existing_controller : _controllers)
	{
		if (!existing_controller)
		{
			continue;
		}

		SDL_Joystick* existing_joystick = SDL_GameControllerGetJoystick(existing_controller);
		if (SDL_JoystickInstanceID(existing_joystick) == joystick_id)
		{
			SDL_GameControllerClose(controller);
			return;
		}
	}

	_controllers.push_back(controller);
}

void Application::close_controller(SDL_JoystickID joystick_id)
{
	std::vector<SDL_GameController*>::iterator iter = std::remove_if(
		_controllers.begin(),
		_controllers.end(),
		[joystick_id](SDL_GameController* controller)
		{
			if (!controller)
			{
				return true;
			}

			SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
			if (SDL_JoystickInstanceID(joystick) != joystick_id)
			{
				return false;
			}

			SDL_GameControllerClose(controller);
			return true;
		}
	);

	_controllers.erase(iter, _controllers.end());
}

void Application::close_all_controllers()
{
	for (SDL_GameController* controller : _controllers)
	{
		if (controller)
		{
			SDL_GameControllerClose(controller);
		}
	}

	_controllers.clear();
}

void Application::handle_controller_device_event(const SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_CONTROLLERDEVICEADDED:
		open_controller(event.cdevice.which);
		break;

	case SDL_CONTROLLERDEVICEREMOVED:
		close_controller(event.cdevice.which);
		break;

	default:
		break;
	}
}


