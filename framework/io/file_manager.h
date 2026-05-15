#pragma once
#include "json_loader.h"

#include "../resources/resource_types.h"

#include<vector>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>

class FileManager
{
public:
	bool load_assets_strcutre(const std::filesystem::path& assets_structure_path);
	bool load_configs();
	bool load_textures();
	bool load_audio();
	bool load_i18n();

	const std::vector<AtlasLoadRequest>& atlas_load_requests() const;
	const std::vector<AnimationBuildRequest>& animation_build_requests() const;

private:
	bool load_character_manifest(const std::filesystem::path& manifest_path);
	bool load_character_config(
		const std::string& manifest_id,
		const std::string& manifest_asset_key,
		const std::filesystem::path& config_path
	);
	bool load_animation_config(
		const std::string& character_id,
		const std::filesystem::path& texture_root,
		const std::filesystem::path& animation_config_path
	);
	bool append_animation_clip(
		const std::string& character_id,
		const std::string& animation_name,
		bool is_segment,
		size_t segment_index,
		const json& clip_node,
		const std::filesystem::path& texture_root
	);
	std::filesystem::path resolve_asset_path(
		const std::filesystem::path& path
	) const;

private:
	std::vector<std::pair<std::string, std::string>> _directories;
	std::unordered_map<std::string, std::filesystem::path> _manifest_paths;
	std::vector<AtlasLoadRequest> _atlas_load_requests;
	std::vector<AnimationBuildRequest> _animation_build_requests;
	std::filesystem::path _assets_root;
	std::filesystem::path _project_root;
	bool _find_all_folder = false;
};
