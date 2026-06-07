#pragma once

#include "../../engine/scene/scene.h"

class MainMenuScene final : public Scene
{
public:
	MainMenuScene() = default;
	~MainMenuScene() override = default;

	void on_update(double delta)override;
	void on_render(SDL_Renderer* renderer)override;
	void on_input(const InputSnapshot& input, const std::vector<InputEvent>& events)override;

	void on_enter(const ScenePayload& payload) override;
	void on_exit() override;
	void reset() override;
};
