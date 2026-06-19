#include "atlas_manager.h"

#include "atlas_builder.h"
#include "../texture/texture_loader.h"
#include "../texture/texture_manager.h"

#include <iostream>
#include <utility>

AtlasManager::AtlasManager(TextureManager& texture_manager)
	: _texture_manager(texture_manager)
{
}

bool AtlasManager::begin_build(const AtlasBuildRequest& request)
{
	if (!request.is_valid())
	{
		std::cout << "Begin atlas build failed: request is invalid: "
			<< request.atlas_key << std::endl;
		return false;
	}

	if (_atlas_pool.contains(request.atlas_key))
	{
		std::cout << "Begin atlas build failed: atlas already exists: "
			<< request.atlas_key << std::endl;
		return false;
	}

	if (_assembly_states.contains(request.atlas_key))
	{
		std::cout << "Begin atlas build failed: atlas build already exists: "
			<< request.atlas_key << std::endl;
		return false;
	}

	AtlasAssemblyState state;
	state.request = request;
	state.committed_frames.resize(request.frame_count);
	state.committed_frame_count = 0;
	state.finalized = false;
	_assembly_states.emplace(request.atlas_key, std::move(state));
	return true;
}

bool AtlasManager::begin_builds(const std::vector<AtlasBuildRequest>& requests)
{
	for (const AtlasBuildRequest& request : requests)
	{
		if (!begin_build(request))
			return false;
	}

	return true;
}

bool AtlasManager::commit_prepared_frame(
	SDL_Renderer* renderer,
	const AtlasFramePreparedResult& prepared_result
)
{
	if (!renderer)
	{
		std::cout << "Commit atlas frame failed: renderer is null: "
			<< prepared_result.task.atlas_key << std::endl;
		return false;
	}

	if (prepared_result.task.atlas_key.empty())
	{
		std::cout << "Commit atlas frame failed: atlas key is empty." << std::endl;
		return false;
	}

	std::unordered_map<std::string, AtlasAssemblyState>::iterator iterator =
		_assembly_states.find(prepared_result.task.atlas_key);
	if (iterator == _assembly_states.end())
	{
		std::cout << "Commit atlas frame failed: build state does not exist: "
			<< prepared_result.task.atlas_key << std::endl;
		return false;
	}

	AtlasAssemblyState& state = iterator->second;
	if (state.finalized)
	{
		std::cout << "Commit atlas frame failed: atlas already finalized: "
			<< prepared_result.task.atlas_key << std::endl;
		return false;
	}

	if (prepared_result.task.expected_frame_count != state.request.frame_count)
	{
		std::cout << "Commit atlas frame failed: frame count mismatch: "
			<< prepared_result.task.atlas_key << std::endl;
		return false;
	}

	if (prepared_result.task.frame_index >= state.committed_frames.size())
	{
		std::cout << "Commit atlas frame failed: frame index out of range: "
			<< prepared_result.task.atlas_key << ", frame "
			<< prepared_result.task.frame_index << std::endl;
		return false;
	}

	const SurfaceLoadResult& surface_result = prepared_result.surface_result;
	if (!surface_result._success || !surface_result._surface)
	{
		std::cout << "Commit atlas frame failed: prepared surface is invalid: "
			<< prepared_result.task.frame_path << std::endl;
		return false;
	}

	if (surface_result._asset_key != prepared_result.task.atlas_key)
	{
		std::cout << "Commit atlas frame failed: asset key mismatch: "
			<< surface_result._asset_key << std::endl;
		return false;
	}

	if (surface_result._frame_index != prepared_result.task.frame_index)
	{
		std::cout << "Commit atlas frame failed: frame index mismatch: "
			<< prepared_result.task.atlas_key << std::endl;
		return false;
	}

	AtlasAssemblyFrame& frame_state =
		state.committed_frames[prepared_result.task.frame_index];
	if (frame_state.committed)
	{
		std::cout << "Commit atlas frame failed: frame already committed: "
			<< prepared_result.task.atlas_key << ", frame "
			<< prepared_result.task.frame_index << std::endl;
		return false;
	}

	TextureLoader texture_loader;
	TextureLoadResult texture_result =
		texture_loader.load_texture(renderer, surface_result);
	if (!texture_result._success || !texture_result._texture)
		return false;

	const std::string texture_key = make_texture_key(
		prepared_result.task.atlas_key,
		prepared_result.task.frame_index
	);
	if (!_texture_manager.store_texture(texture_key, std::move(texture_result._texture)))
		return false;

	frame_state.frame_path = surface_result._frame_path;
	frame_state.texture = _texture_manager.find_texture(texture_key);
	frame_state.committed = true;
	++state.committed_frame_count;

	if (!frame_state.texture)
	{
		std::cout << "Commit atlas frame failed: stored texture lookup failed: "
			<< texture_key << std::endl;
		return false;
	}

	if (state.committed_frame_count == state.request.frame_count)
		return finalize_build(state.request.atlas_key);

	return true;
}

Atlas* AtlasManager::find_atlas(const std::string_view& key) const
{
	AtlasPool::const_iterator iterator = _atlas_pool.find(std::string(key));
	if (iterator == _atlas_pool.end())
		return nullptr;

	return iterator->second.get();
}

bool AtlasManager::has_in_progress_build(const std::string_view& key) const
{
	return _assembly_states.contains(std::string(key));
}

size_t AtlasManager::in_progress_build_count() const
{
	return _assembly_states.size();
}

void AtlasManager::clear()
{
	_assembly_states.clear();
	_atlas_pool.clear();
}

size_t AtlasManager::resource_count() const
{
	return _atlas_pool.size();
}

bool AtlasManager::finalize_build(const std::string& atlas_key)
{
	std::unordered_map<std::string, AtlasAssemblyState>::iterator iterator =
		_assembly_states.find(atlas_key);
	if (iterator == _assembly_states.end())
	{
		std::cout << "Finalize atlas build failed: build state does not exist: "
			<< atlas_key << std::endl;
		return false;
	}

	AtlasAssemblyState& state = iterator->second;
	if (state.finalized)
	{
		std::cout << "Finalize atlas build failed: atlas already finalized: "
			<< atlas_key << std::endl;
		return false;
	}

	if (state.committed_frame_count != state.request.frame_count)
	{
		std::cout << "Finalize atlas build failed: committed frame count mismatch: "
			<< atlas_key << ", expected " << state.request.frame_count
			<< ", actual " << state.committed_frame_count << std::endl;
		return false;
	}

	std::vector<AtlasCommittedFrame> committed_frames;
	committed_frames.reserve(state.committed_frames.size());
	for (size_t index = 0; index < state.committed_frames.size(); ++index)
	{
		const AtlasAssemblyFrame& frame_state = state.committed_frames[index];
		if (!frame_state.committed || !frame_state.texture)
		{
			std::cout << "Finalize atlas build failed: frame is missing: "
				<< atlas_key << ", frame " << index << std::endl;
			return false;
		}

		AtlasCommittedFrame committed_frame;
		committed_frame.frame_path = frame_state.frame_path;
		committed_frame.texture = frame_state.texture;
		committed_frame.frame_index = index;
		committed_frames.push_back(std::move(committed_frame));
	}

	std::unique_ptr<Atlas> atlas = std::make_unique<Atlas>(atlas_key);
	AtlasBuilder atlas_builder;
	if (!atlas_builder.build_atlas(state.request, committed_frames, *atlas))
		return false;

	state.finalized = true;
	_atlas_pool.emplace(atlas_key, std::move(atlas));
	_assembly_states.erase(iterator);
	return true;
}

std::string AtlasManager::make_texture_key(
	const std::string& atlas_key,
	size_t frame_index
) const
{
	return atlas_key + "#" + std::to_string(frame_index);
}
