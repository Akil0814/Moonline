#pragma once
#include "../../framework/scene/scene.h"
#include "../../framework/ui/progress_bar.h"
#include <thread>
#include <atomic>

//TODO Befor: adding Observer/adding ResourceManager
// 观察 ResourceManager
//LoadingScene 又被别的 UI 观察

class LoadingScene : public Scene
{
public:
	void on_enter()override;
	void on_exit()override;

	void on_update(double delta)override;
	void on_render(SDL_Renderer* renderer)override;
	void on_input(
		const InputSnapshot& input,
		const std::vector<InputEvent>& events
	)override;

	void reset() override;
private:
	ProgressBar load_bar;
};
