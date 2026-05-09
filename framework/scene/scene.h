#pragma once
#include<SDL.h>
#include<vector>

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void on_enter() = 0;
	virtual void on_exit() = 0;

	virtual void on_update(double delta) = 0;
	virtual void on_render(SDL_Renderer* renderer) = 0;
	virtual void on_input(const SDL_Event& event) = 0;\

private:

};

