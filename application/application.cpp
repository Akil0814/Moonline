#include "application.h"

#include "../framework/ui/progress_bar.h"//test
#include "../framework/scene/scene_manager.h"

#include "../framework/resources/resource_bootstrapper.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <filesystem>

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

	_window = SDL_CreateWindow("Moonline", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
	init_assert(_window, "SDL_CreateWindow Error");

	if (SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
	{
		SDL_Log("Failed to enter fullscreen: %s", SDL_GetError());
		SDL_ClearError();

		//fallback
		SDL_SetWindowSize(_window, 1280, 720);
		SDL_SetWindowPosition(_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);//硬件加速 垂直同步 目标纹理
	init_assert(_renderer, "SDL_CreateRenderer Error");

	init_assert(SDL_RenderSetLogicalSize(_renderer, 1280, 720) == 0, "SDL_RenderSetLogicalSize Error");
}


Application:: ~Application()
{
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);

	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool Application::init()
{
	std::cout << "cwd: " << std::filesystem::current_path() << '\n';
	ResourceBootstrapper::instance()->bootstrap(std::filesystem::current_path());
	
	ProgressBar load_bar({ 1000.0f, 680.0f }, { 200.0f, 5.0f });
	//-----------------------------testing-----------------
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
			for (int i = 1; i <= 100; ++i)
			{
				//std::cout << "loading...\n";

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				progress_count.store(i, std::memory_order_relaxed);
			}

			loading_finished.store(true, std::memory_order_release);
		};

	std::thread loading_thread(loading_function);

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

		float progress = progress_count.load(std::memory_order_relaxed) / 100.0f;
		load_bar.set_progress(progress);

		SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
		SDL_RenderClear(_renderer);

		load_bar.on_render(_renderer);

		SDL_RenderPresent(_renderer);
	}

	std::cout << "good" << std::endl;
	//----------------------testing------------------------

	if (loading_thread.joinable())
		loading_thread.join();

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

	while (_active)
	{
		while (SDL_PollEvent(&_event))
		{
			if (_event.type == SDL_QUIT)
				_active = false;
			SceneManager::instance()->on_input(_event);
		}

		Uint64 current_counter = SDL_GetPerformanceCounter();//实现动态延时
		double delta = (double)(current_counter - last_counter) / counter_freq;
		last_counter = current_counter;

		if (delta * 1000 < 1000.0 / FPS)
			SDL_Delay((Uint32)(1000.0 / FPS - delta * 1000));
		
		SceneManager::instance()->on_update(delta);

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
	//...
}


