#pragma once
#include<SDL.h>
#include"scene.h"
#include"../base/singleton.h"

class SceneManager :public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;
protected:

	SceneManager() = default;

	~SceneManager() = default;

public:
	void set_current_scene(Scene* scene);
	void switch_to(Scene* scene);

	void on_update(double delta);
	void on_render(SDL_Renderer* renderer);
	void on_input(const SDL_Event& event);

private:
	Scene* _current_scene = nullptr;
};

