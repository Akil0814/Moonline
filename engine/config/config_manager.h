#pragma once

#include "../io/loaders/asset_config_types.h"
#include "../resources/resource_types.h"
#include <filesystem>
#include <unordered_map>
#include <vector>

#include "../tools/singleton.h"

class ConfigManager : public Singleton<ConfigManager>
{
	friend Singleton<ConfigManager>;

public:
	ConfigManager() = default;
	~ConfigManager() = default;

	bool load_assets_structure(const std::filesystem::path& assets_structure_path);
	bool load_character_resource_requests();
	void clear();

	const std::vector<AtlasLoadRequest>& atlas_load_requests() const;
	const std::vector<AnimationBuildRequest>& animation_build_requests() const;

private:
	bool load_character_resource_requests(
		const std::filesystem::path& manifest_path,
		const std::filesystem::path& layout_path
	);

private:
	std::vector<AssetDirectoryEntry> _directories;
	std::unordered_map<std::string, std::filesystem::path> _manifest_paths;
	std::vector<AtlasLoadRequest> _atlas_load_requests;
	std::vector<AnimationBuildRequest> _animation_build_requests;
	bool _has_assets_structure = false;
};
