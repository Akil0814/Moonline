#include "main_menu_scene.h"

#include "../../application/scene/scene_payloads.h"
#include "../../engine/resources/resource_manager.h"

void MainMenuScene::on_enter(const ScenePayload& payload)
{
	if (payload.has_value())
	{
		const MainMenuEnterPayload& enter_payload =
			require_scene_payload<MainMenuEnterPayload>(payload);
		(void)enter_payload;
	}

	_paused = false;
}

void MainMenuScene::on_update(double delta)
{
	Scene::on_update(delta);
}

void MainMenuScene::on_render(SDL_Renderer* renderer)
{
	Scene::on_render(renderer);
	//---------------test--------------------
	//---------------test--------------------
}

void MainMenuScene::on_input(const InputSnapshot& input, const std::vector<InputEvent>& events)
{
	Scene::on_input(input, events);
}


void MainMenuScene::on_exit()
{
	_paused = false;
}

void MainMenuScene::reset()
{
	_paused = false;
}
