#include "game_content_loader.h"

#include "config_load_pipeline.h"
#include "resource_request_assembler.h"
#include "../animation/animation_manager.h"
#include "../io/path/path_manager.h"
#include "../resources/resource_manager.h"

#include <filesystem>
#include <iostream>
#include <utility>

void GameContentLoader::reset()
{
	_renderer = nullptr;
	_load_plan.clear();
	_error_message.clear();
	_state = GameContentLoaderState::Idle;
	_progress = 0.0f;
}

bool GameContentLoader::start(SDL_Renderer* renderer)
{
	reset();

	if (!renderer)
	{
		fail("GameContentLoader start failed: renderer is null.");
		return false;
	}

	_renderer = renderer;
	_state = GameContentLoaderState::PreparingRequests;


	PathManager* path_manager = PathManager::instance();
	if (!path_manager->is_initialized())
	{
		fail("GameContentLoader start failed: path manager is not init.");
		return false;
	}

	ConfigLoadPipeline config_load_pipeline;
	ConfigLoadResult config_result;
	const std::filesystem::path assets_structure_path = path_manager->assets_structure();
	if (!config_load_pipeline.load(assets_structure_path, config_result))
	{
		fail(config_load_pipeline.error_message());
		return false;
	}

	ResourceRequestAssembler assembler;
	if (!assembler.assemble(config_result, _load_plan))
	{
		fail("GameContentLoader start failed: resource request assembly failed.");
		return false;
	}

	_state = GameContentLoaderState::LoadingResources;
	_progress = 0.0f;
	return true;
}

void GameContentLoader::update()
{
	if (_state != GameContentLoaderState::LoadingResources)
		return;

	if (!_renderer)
	{
		fail("GameContentLoader update failed: renderer is null.");
		return;
	}

	//_state = GameContentLoaderState::Finished;
	_progress = 1.0f;
}

bool GameContentLoader::is_running() const
{
	return _state == GameContentLoaderState::PreparingRequests
		|| _state == GameContentLoaderState::LoadingResources;
}

bool GameContentLoader::is_finished() const
{
	return _state == GameContentLoaderState::Finished;
}

bool GameContentLoader::has_failed() const
{
	return _state == GameContentLoaderState::Failed;
}

float GameContentLoader::progress() const
{
	return _progress;
}

const std::string& GameContentLoader::error_message() const
{
	return _error_message;
}

GameContentLoaderState GameContentLoader::state() const
{
	return _state;
}

void GameContentLoader::fail(std::string message)
{
	_error_message = std::move(message);
	_state = GameContentLoaderState::Failed;
	std::cout << _error_message << std::endl;
}
