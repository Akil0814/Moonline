#include "game_content_loader.h"

#include "config_load_pipeline.h"
#include "resource_request_assembler.h"
#include "../animation/animation_manager.h"
#include "../animation/effect_manager.h"
#include "../io/path/path_manager.h"
#include "../resources/resource_manager.h"
#include "../resources/texture/surface_loader.h"
#include "../resources/texture/texture_loader.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

namespace
{
constexpr std::size_t kMaxInFlightPrepareJobs = 16;
constexpr std::size_t kTextureCommitBudgetPerUpdate = 4;
constexpr std::size_t kAtlasFrameCommitBudgetPerUpdate = 8;

std::size_t resolve_worker_count(std::size_t total_prepare_jobs)
{
	if (total_prepare_jobs == 0)
		return 0;

	std::size_t worker_count = std::thread::hardware_concurrency();
	if (worker_count == 0)
		worker_count = 2;

	worker_count = std::min<std::size_t>(worker_count, 4);
	return std::min(worker_count, total_prepare_jobs);
}
}

GameContentLoader::~GameContentLoader()
{
	shutdown_worker_threads();
}

void GameContentLoader::reset()
{
	shutdown_worker_threads();
	reset_streaming_state();
	_renderer = nullptr;
	_load_plan.clear();
	_error_message.clear();
	_state = GameContentLoaderState::Idle;
	_progress = 0.0f;
	_total_work_units = 0;
	_completed_work_units = 0;
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

	if (!initialize_streaming_work())
		return false;

	_state = GameContentLoaderState::StreamingTextureAndAtlasWork;
	_progress = 0.0f;

	std::cout << "Total requests built: " << _load_plan.total_request_count()
		<< ", total work units: " << _total_work_units << std::endl;
	return true;
}

void GameContentLoader::update()
{
	if (_state == GameContentLoaderState::Idle
		|| _state == GameContentLoaderState::Finished
		|| _state == GameContentLoaderState::Failed)
	{
		return;
	}

	if (_state == GameContentLoaderState::StreamingTextureAndAtlasWork)
	{
		if (!_renderer)
		{
			fail("GameContentLoader update failed: renderer is null.");
			return;
		}

		dispatch_prepare_jobs();
		drain_completed_prepare_results();
		if (!commit_ready_streaming_results())
			return;

		if (is_streaming_phase_complete())
		{
			shutdown_worker_threads();
			_state = GameContentLoaderState::LoadingFonts;
		}

		update_progress_value();
		return;
	}

	if (_state == GameContentLoaderState::LoadingFonts)
	{
		if (!load_fonts())
			return;

		_state = GameContentLoaderState::LoadingAudio;
		update_progress_value();
		return;
	}

	if (_state == GameContentLoaderState::LoadingAudio)
	{
		if (!load_audio())
			return;

		_state = GameContentLoaderState::RegisteringAnimations;
		update_progress_value();
		return;
	}

	if (_state == GameContentLoaderState::RegisteringAnimations)
	{
		if (!register_animations())
			return;

		_state = GameContentLoaderState::RegisteringEffects;
		update_progress_value();
		return;
	}

	if (_state == GameContentLoaderState::RegisteringEffects)
	{
		if (!register_effects())
			return;

		_state = GameContentLoaderState::Finished;
		_progress = 1.0f;
	}
}

bool GameContentLoader::is_running() const
{
	return _state == GameContentLoaderState::PreparingRequests
		|| _state == GameContentLoaderState::StreamingTextureAndAtlasWork
		|| _state == GameContentLoaderState::LoadingFonts
		|| _state == GameContentLoaderState::LoadingAudio
		|| _state == GameContentLoaderState::RegisteringAnimations
		|| _state == GameContentLoaderState::RegisteringEffects;
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

bool GameContentLoader::initialize_streaming_work()
{
	AtlasBuildPreparer atlas_build_preparer;
	std::vector<AtlasFramePrepareTask> atlas_frame_tasks;
	for (const AtlasBuildRequest& request : _load_plan.atlas_build_requests())
	{
		std::vector<AtlasFramePrepareTask> expanded_tasks;
		if (!atlas_build_preparer.expand_build_request(request, expanded_tasks))
		{
			fail("GameContentLoader start failed: atlas build request expansion failed.");
			return false;
		}

		atlas_frame_tasks.insert(
			atlas_frame_tasks.end(),
			std::make_move_iterator(expanded_tasks.begin()),
			std::make_move_iterator(expanded_tasks.end())
		);
	}

	ResourceManager* resource_manager = ResourceManager::instance();
	if (!resource_manager->begin_atlas_builds(_load_plan.atlas_build_requests()))
	{
		fail("GameContentLoader start failed: atlas build initialization failed.");
		return false;
	}

	_atlas_frame_tasks = std::move(atlas_frame_tasks);
	_total_work_units = _load_plan.texture_requests().size()
		+ _atlas_frame_tasks.size()
		+ _load_plan.font_requests().size()
		+ _load_plan.sound_requests().size()
		+ _load_plan.music_requests().size()
		+ _load_plan.animation_build_requests().size()
		+ _load_plan.effect_build_requests().size();

	start_worker_threads();
	return true;
}

void GameContentLoader::reset_streaming_state()
{
	_atlas_frame_tasks.clear();
	_next_texture_request_index = 0;
	_next_atlas_frame_task_index = 0;
	_dispatch_texture_turn = true;
	_prepare_jobs.clear();
	_completed_texture_results.clear();
	_completed_atlas_frame_results.clear();
	_ready_texture_results.clear();
	_ready_atlas_frame_results.clear();
	_in_flight_prepare_job_count.store(0);
	_stop_workers.store(false);
	_prepared_texture_count = 0;
	_prepared_atlas_frame_count = 0;
	_committed_texture_count = 0;
	_committed_atlas_frame_count = 0;
}

void GameContentLoader::start_worker_threads()
{
	const std::size_t total_prepare_jobs =
		_load_plan.texture_requests().size() + _atlas_frame_tasks.size();
	const std::size_t worker_count = resolve_worker_count(total_prepare_jobs);
	if (worker_count == 0)
		return;

	_worker_threads.reserve(worker_count);
	for (std::size_t index = 0; index < worker_count; ++index)
		_worker_threads.emplace_back(&GameContentLoader::worker_loop, this);
}

void GameContentLoader::shutdown_worker_threads()
{
	_stop_workers.store(true);
	_prepare_cv.notify_all();

	for (std::thread& worker : _worker_threads)
	{
		if (worker.joinable())
			worker.join();
	}
	_worker_threads.clear();
}

void GameContentLoader::worker_loop()
{
	SurfaceLoader surface_loader;
	AtlasBuildPreparer atlas_build_preparer;

	for (;;)
	{
		PrepareJob job;
		{
			std::unique_lock<std::mutex> lock(_prepare_mutex);
			_prepare_cv.wait(lock, [this]()
			{
				return _stop_workers.load() || !_prepare_jobs.empty();
			});

			if (_stop_workers.load() && _prepare_jobs.empty())
				return;

			job = std::move(_prepare_jobs.front());
			_prepare_jobs.pop_front();
		}

		if (std::holds_alternative<TextureLoadRequest>(job.payload))
		{
			const TextureLoadRequest& texture_request =
				std::get<TextureLoadRequest>(job.payload);

			SurfaceLoadRequest surface_request;
			surface_request._asset_key = texture_request.key;
			surface_request._frame_path = texture_request.file_path;
			surface_request._frame_index = 0;

			SurfaceLoadResult surface_result =
				surface_loader.load_surface(surface_request);
			{
				std::lock_guard<std::mutex> lock(_completed_results_mutex);
				_completed_texture_results.push_back(std::move(surface_result));
			}
		}
		else
		{
			AtlasFramePreparedResult prepared_result =
				atlas_build_preparer.prepare_frame(
					std::get<AtlasFramePrepareTask>(job.payload)
				);
			{
				std::lock_guard<std::mutex> lock(_completed_results_mutex);
				_completed_atlas_frame_results.push_back(std::move(prepared_result));
			}
		}

		_in_flight_prepare_job_count.fetch_sub(1);
	}
}

void GameContentLoader::dispatch_prepare_jobs()
{
	auto try_enqueue_texture = [this]() -> bool
	{
		if (_next_texture_request_index >= _load_plan.texture_requests().size())
			return false;

		PrepareJob job;
		job.payload = _load_plan.texture_requests()[_next_texture_request_index++];
		{
			std::lock_guard<std::mutex> lock(_prepare_mutex);
			_prepare_jobs.push_back(std::move(job));
		}

		_in_flight_prepare_job_count.fetch_add(1);
		_prepare_cv.notify_one();
		return true;
	};

	auto try_enqueue_atlas = [this]() -> bool
	{
		if (_next_atlas_frame_task_index >= _atlas_frame_tasks.size())
			return false;

		PrepareJob job;
		job.payload = _atlas_frame_tasks[_next_atlas_frame_task_index++];
		{
			std::lock_guard<std::mutex> lock(_prepare_mutex);
			_prepare_jobs.push_back(std::move(job));
		}

		_in_flight_prepare_job_count.fetch_add(1);
		_prepare_cv.notify_one();
		return true;
	};

	while (_in_flight_prepare_job_count.load() < kMaxInFlightPrepareJobs)
	{
		bool enqueued = false;
		if (_dispatch_texture_turn)
			enqueued = try_enqueue_texture() || try_enqueue_atlas();
		else
			enqueued = try_enqueue_atlas() || try_enqueue_texture();

		if (!enqueued)
			break;

		_dispatch_texture_turn = !_dispatch_texture_turn;
	}
}

void GameContentLoader::drain_completed_prepare_results()
{
	std::lock_guard<std::mutex> lock(_completed_results_mutex);

	while (!_completed_texture_results.empty())
	{
		_ready_texture_results.push_back(std::move(_completed_texture_results.front()));
		_completed_texture_results.pop_front();
		++_prepared_texture_count;
	}

	while (!_completed_atlas_frame_results.empty())
	{
		_ready_atlas_frame_results.push_back(
			std::move(_completed_atlas_frame_results.front())
		);
		_completed_atlas_frame_results.pop_front();
		++_prepared_atlas_frame_count;
	}
}

bool GameContentLoader::commit_ready_streaming_results()
{
	std::size_t committed_texture_count = 0;
	while (committed_texture_count < kTextureCommitBudgetPerUpdate
		&& !_ready_texture_results.empty())
	{
		SurfaceLoadResult surface_result = std::move(_ready_texture_results.front());
		_ready_texture_results.pop_front();
		if (!commit_texture_result(surface_result))
			return false;

		++committed_texture_count;
	}

	std::size_t committed_atlas_count = 0;
	while (committed_atlas_count < kAtlasFrameCommitBudgetPerUpdate
		&& !_ready_atlas_frame_results.empty())
	{
		AtlasFramePreparedResult prepared_result =
			std::move(_ready_atlas_frame_results.front());
		_ready_atlas_frame_results.pop_front();
		if (!commit_atlas_frame_result(prepared_result))
			return false;

		++committed_atlas_count;
	}

	return true;
}

bool GameContentLoader::commit_texture_result(const SurfaceLoadResult& surface_result)
{
	if (!surface_result._success || !surface_result._surface)
	{
		fail("GameContentLoader texture commit failed: prepared surface is invalid.");
		return false;
	}

	TextureLoader texture_loader;
	TextureLoadResult texture_result =
		texture_loader.load_texture(_renderer, surface_result);
	if (!texture_result._success || !texture_result._texture)
	{
		fail("GameContentLoader texture commit failed: texture creation failed.");
		return false;
	}

	ResourceManager* resource_manager = ResourceManager::instance();
	if (!resource_manager->texture_manager().store_texture(
		surface_result._asset_key,
		std::move(texture_result._texture)))
	{
		fail("GameContentLoader texture commit failed: texture store failed.");
		return false;
	}

	++_committed_texture_count;
	++_completed_work_units;
	return true;
}

bool GameContentLoader::commit_atlas_frame_result(
	const AtlasFramePreparedResult& prepared_result
)
{
	if (!prepared_result.surface_result._success || !prepared_result.surface_result._surface)
	{
		fail("GameContentLoader atlas frame commit failed: prepared surface is invalid.");
		return false;
	}

	if (!ResourceManager::instance()->commit_prepared_atlas_frame(
		_renderer,
		prepared_result))
	{
		fail("GameContentLoader atlas frame commit failed: atlas manager commit failed.");
		return false;
	}

	++_committed_atlas_frame_count;
	++_completed_work_units;
	return true;
}

bool GameContentLoader::is_streaming_phase_complete()
{
	if (_next_texture_request_index != _load_plan.texture_requests().size())
		return false;

	if (_next_atlas_frame_task_index != _atlas_frame_tasks.size())
		return false;

	if (_committed_texture_count != _load_plan.texture_requests().size())
		return false;

	if (_committed_atlas_frame_count != _atlas_frame_tasks.size())
		return false;

	if (_in_flight_prepare_job_count.load() != 0)
		return false;

	if (!_ready_texture_results.empty() || !_ready_atlas_frame_results.empty())
		return false;

	{
		std::lock_guard<std::mutex> lock(_completed_results_mutex);
		if (!_completed_texture_results.empty()
			|| !_completed_atlas_frame_results.empty())
		{
			return false;
		}
	}

	return ResourceManager::instance()->atlas_manager().in_progress_build_count() == 0;
}

bool GameContentLoader::load_fonts()
{
	ResourceManager* resource_manager = ResourceManager::instance();
	for (const FontLoadRequest& request : _load_plan.font_requests())
	{
		if (!resource_manager->load_font(request.key, request.file_path, request.point_size))
		{
			fail("GameContentLoader font load failed.");
			return false;
		}

		++_completed_work_units;
	}

	return true;
}

bool GameContentLoader::load_audio()
{
	ResourceManager* resource_manager = ResourceManager::instance();
	for (const SoundLoadRequest& request : _load_plan.sound_requests())
	{
		if (!resource_manager->audio_manager().load_sound(request.key, request.file_path))
		{
			fail("GameContentLoader sound load failed.");
			return false;
		}

		++_completed_work_units;
	}

	for (const MusicLoadRequest& request : _load_plan.music_requests())
	{
		if (!resource_manager->audio_manager().load_music(request))
		{
			fail("GameContentLoader music load failed.");
			return false;
		}

		++_completed_work_units;
	}

	return true;
}

bool GameContentLoader::register_animations()
{
	ResourceManager* resource_manager = ResourceManager::instance();
	AnimationManager* animation_manager = AnimationManager::instance();
	for (const AnimationBuildRequest& request : _load_plan.animation_build_requests())
	{
		const Atlas* atlas = resource_manager->find_atlas(request.atlas_key);
		if (!animation_manager->register_animation(request, atlas))
		{
			fail("GameContentLoader animation registration failed.");
			return false;
		}

		++_completed_work_units;
	}

	return true;
}

bool GameContentLoader::register_effects()
{
	EffectManager* effect_manager = EffectManager::instance();
	for (const EffectBuildRequest& request : _load_plan.effect_build_requests())
	{
		if (!effect_manager->register_effect(request))
		{
			fail("GameContentLoader effect registration failed.");
			return false;
		}

		++_completed_work_units;
	}

	return true;
}

void GameContentLoader::update_progress_value()
{
	if (_state == GameContentLoaderState::Finished)
	{
		_progress = 1.0f;
		return;
	}

	if (_total_work_units == 0)
	{
		_progress = 0.0f;
		return;
	}

	const float ratio = static_cast<float>(_completed_work_units)
		/ static_cast<float>(_total_work_units);
	_progress = std::clamp(ratio, 0.0f, 1.0f);
}

void GameContentLoader::fail(std::string message)
{
	shutdown_worker_threads();
	_error_message = std::move(message);
	_state = GameContentLoaderState::Failed;
	update_progress_value();
	std::cout << _error_message << std::endl;
}

