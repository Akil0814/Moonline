#include "loading_scene.h"

#include "../../engine/resources/resource_manager.h"

#include <iostream>

void LoadingScene::on_enter()
{
	_paused = false;
	std::cout << "loading scene" << std::endl;
}

void LoadingScene::on_update(double delta)
{

}

void LoadingScene::on_render(SDL_Renderer* renderer)
{
	//---------------test--------------------
	//---------------test--------------------
}

void LoadingScene::on_input(const InputSnapshot& input, const std::vector<InputEvent>& events)
{

}


void LoadingScene::on_exit()
{
	_paused = false;
}

void LoadingScene::reset()
{
	_paused = false;
}
