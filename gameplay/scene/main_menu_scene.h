#pragma once

#include "../../application/scene/application_scene.h"

class MainMenuScene final : public ApplicationScene
{
public:
	MainMenuScene() = default;
	~MainMenuScene() override = default;

	void on_update(double delta)override;
	void on_render(SDL_Renderer* renderer)override;
	void on_input(const RawInputFrame& input, const std::vector<RawInputEvent>& events)override;

	void on_enter(const ScenePayload& payload) override;
	void on_exit() override;
	void reset() override;
};
