#include "main_menu_scene.h"

#include "../../engine/resources/resource_manager.h"

void TestScene::on_enter()
{
	_paused = false;
}

void TestScene::on_update(double delta)
{
	Scene::on_update(delta);
}

void TestScene::on_render(SDL_Renderer* renderer)
{
	Scene::on_render(renderer);
	//---------------test--------------------
	//---------------test--------------------
}

void TestScene::on_input(const InputSnapshot& input, const std::vector<InputEvent>& events)
{

}


void TestScene::on_exit()
{
	_paused = false;
}

void TestScene::reset()
{
	_paused = false;
}
