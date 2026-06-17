#include "game_content_loader.h"

#include "config_load_pipeline.h"
#include "resource_request_assembler.h"
#include "../animation/animation_manager.h"
#include "../config/config_manager.h"
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

	//纭繚璺緞鍚堢悊
	PathManager* path_manager = PathManager::instance();
	if (!path_manager->is_initialized())
	{
		fail("GameContentLoader start failed: path manager is not init.");
		return false;
	}

	//杞藉叆閰嶇疆
	ConfigManager* config_manager = ConfigManager::instance();
	ConfigLoadPipeline config_load_pipeline;
	ConfigLoadResult config_result;
	const std::filesystem::path assets_structure_path = path_manager->assets_structure();
	if (!config_load_pipeline.load(assets_structure_path, config_result))
	{
		fail(config_load_pipeline.error_message());
		return false;
	}

	config_manager->clear();
	config_manager->set_font_manifest(config_result.font_manifest);
	config_manager->set_audio_manifest(config_result.audio_manifest);

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

	ResourceManager* resource_manager = ResourceManager::instance();
	if (!resource_manager->load_atlases(_renderer, _load_plan.atlas_requests()))
	{
		fail("GameContentLoader update failed: atlas loading failed.");
		return;
	}

	if (!AnimationManager::instance()->register_animations(
		_load_plan.animation_build_requests(),
		*resource_manager))
	{
		fail("GameContentLoader update failed: animation registration failed.");
		return;
	}

	_state = GameContentLoaderState::Finished;
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
