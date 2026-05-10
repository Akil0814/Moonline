#pragma once
#include "../../framework/scene/scene.h"

class LoadingScene : public Scene
{
public:
	void on_enter()override;
	void on_exit()override;

	void on_update(double delta)override;
	void on_render(SDL_Renderer* renderer)override;
	void on_input(const SDL_Event& event)override;
private:

};