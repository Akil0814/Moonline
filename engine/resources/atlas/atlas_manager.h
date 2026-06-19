#pragma once
#include "atlas.h"
#include "atlas_build_preparer.h"
#include "../resource_sub_manager.h"
#include "../resource_types.h"

#include <SDL.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class TextureManager;

using AtlasPool = std::unordered_map<std::string, std::unique_ptr<Atlas>>;

class AtlasManager : public ResourceSubManager
{
public:
	explicit AtlasManager(TextureManager& texture_manager);

	bool begin_build(const AtlasBuildRequest& request);
	bool begin_builds(const std::vector<AtlasBuildRequest>& requests);
	bool commit_prepared_frame(
		SDL_Renderer* renderer,
		const AtlasFramePreparedResult& prepared_result
	);

	Atlas* find_atlas(const std::string_view& key) const;
	bool has_in_progress_build(const std::string_view& key) const;
	size_t in_progress_build_count() const;

	void clear() override;
	size_t resource_count() const override;

private:
	struct AtlasAssemblyFrame
	{
		std::filesystem::path frame_path;
		SDL_Texture* texture = nullptr;
		bool committed = false;
	};

	struct AtlasAssemblyState
	{
		AtlasBuildRequest request;
		std::vector<AtlasAssemblyFrame> committed_frames;
		size_t committed_frame_count = 0;
		bool finalized = false;
	};

	bool finalize_build(const std::string& atlas_key);
	std::string make_texture_key(
		const std::string& atlas_key,
		size_t frame_index
	) const;

private:
	TextureManager& _texture_manager;
	AtlasPool _atlas_pool;
	std::unordered_map<std::string, AtlasAssemblyState> _assembly_states;
};
