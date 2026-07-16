#pragma once

#include "../../engine/scene/scene.h"
#include "../../engine/loading/game_content_loader.h"
#include "../../engine/ui/widgets/image/ui_fade_image.h"
#include "../../engine/ui/widgets/ui_bar.h"
#include "../../engine/ui/widgets/image/ui_blink_image.h"

namespace arcneco::scene
{
class StartupLoadingScene final : public elysia::scene::Scene
{
public:
	StartupLoadingScene() = default;
	~StartupLoadingScene() override = default;

	void on_update(double delta) override;
	void on_render(SDL_Renderer* renderer) override;
	void on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events) override;

	void on_enter(const elysia::scene::ScenePayload& payload) override;
	void on_exit() override;
	void reset() override;

private:
	enum class StartupPhase
	{
		LoadingAndIntro,
		LoadingOnly,
		IntroOnly,
		WaitingForStartInput,
		Transitioning
	};

	void on_loading_finished();
	void on_intro_sequence_finished();
	void enter_waiting_for_start_input();

private:
	elysia::ui::UiBar* _loading_bar = nullptr;
	elysia::ui::UiFadeImage* _akil_icon = nullptr;
	elysia::ui::UiFadeImage* _engine_icon = nullptr;
	elysia::ui::UiBlinkImage* _start_text = nullptr;

	elysia::loading::GameContentLoader _content_loader;

	StartupPhase _phase = StartupPhase::LoadingAndIntro;

	bool _has_logged_load_failure = false;
};
}
