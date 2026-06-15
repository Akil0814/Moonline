#include "startup_loading_scene.h"

#include "../../engine/resources/resource_manager.h"
#include "../../engine/resources/resource_bootstrapper.h"

#include <iostream>

void StartupLoadingScene::on_enter(const ScenePayload& payload)
{
	(void)payload;
	_paused = false;
	std::cout << "loading scene" << std::endl;

	SDL_Texture* akil_tex = ResourceBootstrapper::instance()->get_preload_texture("Akil.png");

	akil_icon = Scene::create_and_add_object<UiFadeImage>(akil_tex, Rect{ 600,300,200,200 });

	akil_icon->configure_playback(UiFadeImageMode::FadeInOut, 2, 2, 2);
	akil_icon->play();
}

void StartupLoadingScene::on_update(double delta)
{
	Scene::on_update(delta);
}

void StartupLoadingScene::on_render(SDL_Renderer* renderer)
{
	Scene::on_render(renderer);
	//---------------test--------------------
	//---------------test--------------------
}

void StartupLoadingScene::on_input(const RawInputFrame& input, const std::vector<RawInputEvent>& events)
{
	ApplicationScene::on_input(input, events);
}


void StartupLoadingScene::on_exit()
{
	_paused = false;
}

void StartupLoadingScene::reset()
{
	_paused = false;
}
