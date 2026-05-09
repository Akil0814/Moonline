#pragma once
#include "../framework/base/singleton.h"
#include <SDL.h>

class Application: public Singleton<Application>
{
    friend Singleton<Application>;
public:
    Application();
    ~Application();

    bool init();
    int run(int argc, char** argv);
    void shutdown();


    void init_assert(bool flag, const char* err_msg)
    {
        if (flag)
            return;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, u8"Game Start Error", err_msg, _window);
        exit(-1);
    }

private:

    const int _logical_width = 1080;
    const int _logical_height = 720;

    double FPS = 60;

    Uint64 _last_counter = 0;
    Uint64 _counter_freq = 0;

    SDL_Event _event;

    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;

    bool _active = { true };

};