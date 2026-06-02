#include "application.h"

#include "../engine/ui/widgets/ui_progress_bar.h"//test
#include "../engine/ui/ui_fade_image.h"//test

#include "../engine/core/time.h"
#include "../engine/core/scene/scene_manager.h"

#include "../engine/io/path/path_manager.h"
#include "../engine/resources/resource_manager.h"
#include "../engine/resources/resource_bootstrapper.h"
#include "../engine/resources/texture/surface_loader.h"
#include "../engine/resources/texture/texture_loader.h"

#include "../gameplay/scene/main_menu_scene.h"

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
	//-----------------------------testing-----------------

	init_assert(argc > 0 && argv && argv[0], "Application start path error");
	init_assert(
		ResourceBootstrapper::instance()->bootstrap(argv[0], _renderer),
		"Resource bootstrap error"
	);

	ResourceManager* resource_manager = ResourceManager::instance();
	init_assert(
		resource_manager->load_font(
			"ui.default",
			PathManager::instance()->fonts() / "IPix.ttf",
			24
		),
		"Default UI font load error"
	);

	//-----------------------------testing-----------------
	SurfaceLoader surface_loader;
	SurfaceLoadRequest preload_surface_request;
	preload_surface_request._asset_key = "preload.Akil";
	preload_surface_request._frame_path = PathManager::instance()->preload_file("Akil.png");
	preload_surface_request._frame_index = 0;

	SurfaceLoadResult preload_surface_result =
		surface_loader.load_surface(preload_surface_request);
	init_assert(preload_surface_result._success, "Preload surface load error");

	TextureLoader texture_loader;
	TextureLoadResult preload_texture_result =
		texture_loader.load_texture(_renderer, preload_surface_result);
	init_assert(preload_texture_result._success, "Preload texture load error");


	UiFadeImage image(preload_texture_result._texture.get(), { 530,270 }, { 250,250 });

	UiProgressBar load_bar({ 1000.0f, 680.0f }, { 200.0f, 5.0f });

	image.configure_playback(UiFadeImageMode::FadeInOut, 2, 1, 1);
	image.play();


	std::atomic<int> progress_count = 0;
	std::atomic<bool> loading_finished = false;

	/*
	relaxed
    只关心 atomic 变量本身
    适合 progress 这种显示用数字

	release
    后台线程写完数据后，用它发布完成信号

	acquire
    主线程读取完成信号，用它确认之前的数据可见

	seq_cst
    默认最安全，先用它也完全可以
	*/

	auto loading_function = [&]()
		{
			for (int i = 1; i <= 20; ++i)
			{
				//std::cout << "loading...\n";

				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				progress_count.store(i*5, std::memory_order_relaxed);
			}

			loading_finished.store(true, std::memory_order_release);
		};

	std::thread loading_thread(loading_function);
	Time::instance()->reset();

	//
	Uint64 last_counter = SDL_GetPerformanceCounter();
	const Uint64 counter_freq = SDL_GetPerformanceFrequency();
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	while (!loading_finished.load(std::memory_order_acquire))
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				loading_finished.store(true, std::memory_order_release);
			}
		}

		Uint64 current_counter = SDL_GetPerformanceCounter();//实现动态延时
		double delta = (double)(current_counter - last_counter) / counter_freq;
		last_counter = current_counter;
		Time::instance()->begin_frame(delta);

		if (delta * 1000 < 1000.0 / FPS)
			SDL_Delay((Uint32)(1000.0 / FPS - delta * 1000));

		image.on_update(Time::instance()->delta());

		float progress = progress_count.load(std::memory_order_relaxed) / 100.0f;
		load_bar.set_progress(progress);

		SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
		SDL_RenderClear(_renderer);

		//test
		load_bar.on_render(_renderer);
		image.on_render(_renderer);

		SDL_RenderPresent(_renderer);
	}

	std::cout << "good" << std::endl;
	//----------------------testing------------------------

	if (loading_thread.joinable())
		loading_thread.join();

	_input_system.set_context(InputContext::UI);
	SceneManager::instance()->switch_to<MainMenuScene>();

    return true;
}

int  Application::run(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	Uint64 last_counter = SDL_GetPerformanceCounter();
	const Uint64 counter_freq = SDL_GetPerformanceFrequency();
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	_counter_freq = SDL_GetPerformanceFrequency();
	_last_counter = SDL_GetPerformanceCounter();
	Time::instance()->reset();


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

		SceneManager::instance()->on_input(
			_input_system.snapshot(),
			_input_system.events()
		);

		Uint64 current_counter = SDL_GetPerformanceCounter();//实现动态延时
		double delta = (double)(current_counter - last_counter) / counter_freq;
		last_counter = current_counter;
		Time::instance()->begin_frame(delta);

		if (delta * 1000 < 1000.0 / FPS)
			SDL_Delay((Uint32)(1000.0 / FPS - delta * 1000));
		

		SceneManager::instance()->on_update(Time::instance()->delta());

		SDL_SetRenderDrawColor(_renderer, 0,0,0,0);
		SDL_RenderClear(_renderer);

		SceneManager::instance()->on_render(_renderer);

		SDL_RenderPresent(_renderer);
	}

	shutdown();

    return 0;
}

void Application::shutdown()
{
	close_all_controllers();
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


