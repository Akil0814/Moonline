#include "startup_loading_scene.h"

#include "../../application/application.h"
#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"
#include "../../engine/bootstrap/bootstrapper.h"

#include <optional>

namespace arcneco::scene
{
void StartupLoadingScene::on_enter(const elysia::scene::ScenePayload& payload)
{
	(void)payload;
	_paused = false;
	_has_logged_load_failure = false;
	_phase = StartupPhase::LoadingAndIntro;


	//icon
	SDL_Texture* akil_tex = elysia::bootstrap::Bootstrapper::instance()->get_preload_texture("Akil_icon_1024.png");
	SDL_Texture* engine_tex = elysia::bootstrap::Bootstrapper::instance()->get_preload_texture("elysia_white_1024.png");
	//SDL_Texture* engine_tex = elysia::bootstrap::Bootstrapper::instance()->get_preload_texture("elysia_1024.png");

	_akil_icon = elysia::scene::Scene::create_and_add_object<elysia::ui::UiFadeImage>(akil_tex, elysia::core::Rect{ 540, 260, 200, 200 });
	_engine_icon = elysia::scene::Scene::create_and_add_object<elysia::ui::UiFadeImage>(engine_tex, elysia::core::Rect{ 540, 260, 200, 200 });

	_engine_icon->set_visible(false);

	_akil_icon->set_on_end([this] {
		_engine_icon->set_visible(true);
		_engine_icon->play(); });

	_engine_icon->set_on_end([this] {
		on_intro_sequence_finished();
	});

	_akil_icon->configure_playback(elysia::ui::effects::UiOpacityFadeMode::FadeInOut, 1, 1, 1);
	_engine_icon->configure_playback(elysia::ui::effects::UiOpacityFadeMode::FadeInOut, 1, 1, 1);

	//loading bar
	_loading_bar = elysia::scene::Scene::create_and_add_object<elysia::ui::UiBar>(elysia::core::Rect{ 20, 695, 1240, 5 });
	elysia::ui::UiBarStyleOverrides loading_bar_style{};
	loading_bar_style.draw_border = true;
	_loading_bar->set_style_overrides(loading_bar_style);
	_loading_bar->set_ratio(0.0);


	//starting text
	SDL_Texture* start_tex = elysia::bootstrap::Bootstrapper::instance()->get_preload_texture("start.png");
	_start_text = elysia::scene::Scene::create_and_add_object<elysia::ui::UiBlinkImage>(
		start_tex,
		elysia::core::Vector2{ 640, 690 },
		elysia::core::Vector2{ 300, 32 },
		elysia::ui::from_center
	);
	_start_text->configure_playback(elysia::ui::effects::UiOpacityBlinkMode::VisibleFirst, 0.0, 0.45, 0.45, std::nullopt);

	_start_text->set_visible(false);


	//starting
	_akil_icon->play();

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
		on_loading_finished();
	}

	if (_content_loader.has_failed() && !_has_logged_load_failure)
	{
		_has_logged_load_failure = true;
		const std::string& error_message = _content_loader.error_message();
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,"Game Start Error",
			error_message.c_str(),nullptr);
		request_quit();
		return;
	}
}

void StartupLoadingScene::on_render(SDL_Renderer* renderer)
{
	elysia::scene::Scene::on_render(renderer);
}

void StartupLoadingScene::on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events)
{
	ApplicationScene::on_input(input, events);

	if (_phase != StartupPhase::WaitingForStartInput)
		return;

	for (const elysia::input::RawInputEvent& event : events)
	{
		if (event.type == elysia::input::RawInputEventType::ControlPressed
			&& elysia::input::matches_control(elysia::input::RawInputControl::AnyControl,event.control))
		{
			_phase = StartupPhase::Transitioning;
			request_scene_switch(AppSceneKeys::MainMenu, MainMeunEnterPayload{ .replay_theme_music = true });
			break;
		}
	}
}

void StartupLoadingScene::on_exit()
{
	_paused = false;
	_content_loader.reset();
}

void StartupLoadingScene::reset()
{
	_paused = false;
	_phase = StartupPhase::LoadingAndIntro;
	_has_logged_load_failure = false;

	_content_loader.reset();
}


void StartupLoadingScene::on_loading_finished()
{
	if (_loading_bar)
	{
		_loading_bar->destroy();
		_loading_bar = nullptr;
	}

	switch (_phase)
	{
	case StartupPhase::LoadingAndIntro:
		_phase = StartupPhase::IntroOnly;
		break;
	case StartupPhase::LoadingOnly:
		enter_waiting_for_start_input();
		break;
	case StartupPhase::IntroOnly:
	case StartupPhase::WaitingForStartInput:
	case StartupPhase::Transitioning:
		break;
	}
}

void StartupLoadingScene::on_intro_sequence_finished()
{
	switch (_phase)
	{
	case StartupPhase::LoadingAndIntro:
		_phase = StartupPhase::LoadingOnly;
		break;
	case StartupPhase::IntroOnly:
		enter_waiting_for_start_input();
		break;
	case StartupPhase::LoadingOnly:
	case StartupPhase::WaitingForStartInput:
	case StartupPhase::Transitioning:
		break;
	}
}

void StartupLoadingScene::enter_waiting_for_start_input()
{
	if (_start_text)
	{
		_start_text->set_visible(true);
		_start_text->play();
	}

	_phase = StartupPhase::WaitingForStartInput;
}
}

