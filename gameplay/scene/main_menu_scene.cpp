#include "main_menu_scene.h"

#include "../../engine/resources/resource_manager.h"

void TestScene::on_enter()
{
	_paused = false;
}

void TestScene::on_update(double delta)
{

}

void TestScene::on_render(SDL_Renderer* renderer)
{
	//---------------test--------------------
	//---------------test--------------------
}

void TestScene::on_input(const InputSnapshot& input, const std::vector<InputEvent>& events)
{

}


void TestScene::on_exit()
{
	clear_objects();
	_paused = false;
}

void TestScene::reset()
{
	clear_objects();
	_paused = false;
}
