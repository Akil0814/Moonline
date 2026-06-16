#pragma once

#include <SDL.h>

#include "resource_load_plan.h"

#include <string>

enum class GameContentLoaderState
{
	Idle,
	PreparingRequests,
	LoadingResources,
	Finished,
	Failed
};

class GameContentLoader
{
public:
	void reset();

	bool start(SDL_Renderer* renderer);
	void update();

	bool is_running() const;
	bool is_finished() const;
	bool has_failed() const;
	float progress() const;
	const std::string& error_message() const;
	GameContentLoaderState state() const;

private:
	void fail(std::string message);

private:
	SDL_Renderer* _renderer = nullptr;
	ResourceLoadPlan _load_plan;
	std::string _error_message;
	GameContentLoaderState _state = GameContentLoaderState::Idle;
	float _progress = 0.0f;
};
