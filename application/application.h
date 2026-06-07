#pragma once
#include "../engine/scene/scene_manager_observer.h"
#include "../engine/tools/singleton.h"
#include "../engine/input/input_system.h"

#include <SDL.h>
#include <vector>

class Application
    : public Singleton<Application>
    , public SceneManagerObserver
{
    friend Singleton<Application>;
public:
    Application();
    ~Application();

    bool init(int argc, char** argv);
    int run(int argc, char** argv);
    void shutdown();
    void on_scene_manager_quit_requested() override;


    void init_assert(bool flag, const char* err_msg)
    {
        if (flag)
            return;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game Start Error", err_msg, _window);
        exit(-1);
    }

private:
    void open_connected_controllers();
    void open_controller(int joystick_index);
    void close_controller(SDL_JoystickID joystick_id);
    void close_all_controllers();
    void handle_controller_device_event(const SDL_Event& event);

    const int _logical_width = 1280;
    const int _logical_height = 720;

    double FPS = 60;

    Uint64 _last_counter = 0;
    Uint64 _counter_freq = 0;

    SDL_Event _event;
    InputSystem _input_system;

    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
    std::vector<SDL_GameController*> _controllers;

    bool _active = { true };
    bool _has_shutdown = false;

};
