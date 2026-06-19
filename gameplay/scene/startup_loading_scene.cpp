#include "startup_loading_scene.h"

#include "../../application/application.h"
#include "../../application/scene/scene_keys.h"
#include "../../engine/bootstrap/bootstrapper.h"

void StartupLoadingScene::on_enter(const ScenePayload& payload)
{
	(void)payload;
	_paused = false;
	_has_logged_load_failure = false;

	SDL_Texture* akil_tex = Bootstrapper::instance()->get_preload_texture("Akil.png");

	_akil_icon = Scene::create_and_add_object<UiFadeImage>(akil_tex, Rect{ 540, 260, 200, 200 });
	_loading_bar = Scene::create_and_add_object<UiBar>(Rect{ 700, 700, 200, 5 });
	_loading_bar->set_draw_border(true);
	_loading_bar->set_ratio(0.0);

	_akil_icon->configure_playback(UiFadeImageMode::FadeInOut, 2, 2, 2);
	_akil_icon->play();

	(void)_content_loader.start(Application::instance()->renderer());
}

void StartupLoadingScene::on_update(double delta)
{
	Scene::on_update(delta);

	_content_loader.update();
	if (_loading_bar)
		_loading_bar->set_ratio(_content_loader.progress());

	if (_content_loader.is_finished())
	{
		request_scene_switch(AppSceneKeys::MainMenu);
		return;
	}

	if (_content_loader.has_failed() && !_has_logged_load_failure)
	{
		_has_logged_load_failure = true;
		const std::string& error_message = _content_loader.error_message();
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"Game Start Error",
			error_message.c_str(),
			nullptr);
		request_quit();
		return;
	}
}

void StartupLoadingScene::on_render(SDL_Renderer* renderer)
{
	Scene::on_render(renderer);
}

void StartupLoadingScene::on_input(const RawInputFrame& input, const std::vector<RawInputEvent>& events)
{
	ApplicationScene::on_input(input, events);
}

void StartupLoadingScene::on_exit()
{
	_paused = false;
	_content_loader.reset();
}

void StartupLoadingScene::reset()
{
	_paused = false;
	_has_logged_load_failure = false;
	_content_loader.reset();
}
