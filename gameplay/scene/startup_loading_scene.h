#pragma once

#include "../../application/scene/application_scene.h"
#include "../../engine/loading/game_content_loader.h"
#include "../../engine/ui/widgets/ui_fade_image.h"
#include "../../engine/ui/widgets/ui_bar.h"

class StartupLoadingScene final : public ApplicationScene
{
public:
	StartupLoadingScene() = default;
	~StartupLoadingScene() override = default;

	void on_update(double delta) override;
	void on_render(SDL_Renderer* renderer) override;
	void on_input(const RawInputFrame& input, const std::vector<RawInputEvent>& events) override;

	void on_enter(const ScenePayload& payload) override;
	void on_exit() override;
	void reset() override;

private:
	UiBar* _loading_bar = nullptr;
	UiFadeImage* _akil_icon = nullptr;
	GameContentLoader _content_loader;
	bool _has_logged_load_failure = false;
};
