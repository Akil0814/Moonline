#include "startup_loading_scene.h"

#include "../../application/application.h"
#include "../../application/scene/scene_keys.h"
#include "../../engine/bootstrap/bootstrapper.h"

namespace arcneco::scene
{
void StartupLoadingScene::on_enter(const elysia::scene::ScenePayload& payload)
{
	(void)payload;
	_paused = false;
	_has_logged_load_failure = false;
	_finished_loading = false;


	SDL_Texture* akil_tex = elysia::bootstrap::Bootstrapper::instance()->get_preload_texture("Akil.png");
	SDL_Texture* engine_tex = elysia::bootstrap::Bootstrapper::instance()->get_preload_texture("elysia_1024.png");

	_akil_icon = elysia::scene::Scene::create_and_add_object<elysia::ui::UiFadeImage>(akil_tex, elysia::core::Rect{ 540, 260, 200, 200 });
	_engine_icon = elysia::scene::Scene::create_and_add_object<elysia::ui::UiFadeImage>(engine_tex, elysia::core::Rect{ 540, 260, 200, 200 });

	_engine_icon->set_visible(false);

	_akil_icon->set_on_end([this] {
		_engine_icon->set_visible(true);
		_engine_icon->play(); });

	_engine_icon->set_on_end([this] {
		_icon_updateing = false;
		});

	_akil_icon->configure_playback(elysia::ui::UiFadeImageMode::FadeInOut, 2, 1, 2);
	_engine_icon->configure_playback(elysia::ui::UiFadeImageMode::FadeInOut, 2, 1, 2);

	_loading_bar = elysia::scene::Scene::create_and_add_object<elysia::ui::UiBar>(elysia::core::Rect{ 20, 700, 1240, 5 });
	_loading_bar->set_draw_border(true);
	_loading_bar->set_ratio(0.0);

	_akil_icon->configure_playback(elysia::ui::UiFadeImageMode::FadeInOut, 1, 1, 1);
	_akil_icon->play();
	_icon_updateing = true;

	(void)_content_loader.start(Application::instance()->renderer());
}

void StartupLoadingScene::on_update(double delta)
{
	elysia::scene::Scene::on_update(delta);

	_content_loader.update();
	if (_loading_bar)
		_loading_bar->set_ratio(_content_loader.progress());

	if (_content_loader.is_finished())
	{
		_finished_loading = true;
		_loading_bar->set_visible(false);
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

	//request_scene_switch(AppSceneKeys::MainMenu);

}

void StartupLoadingScene::on_render(SDL_Renderer* renderer)
{
	elysia::scene::Scene::on_render(renderer);
}

void StartupLoadingScene::on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events)
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
	_finished_loading = false;
	_icon_updateing = false;
	_has_logged_load_failure = false;
	_content_loader.reset();
}
}
