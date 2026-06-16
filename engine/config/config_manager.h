#pragma once

#include "../io/loaders/asset_config_types.h"
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
	bool load_character_animation_content();
	void clear();

	const std::vector<AssetDirectoryEntry>& directories() const;
	const std::unordered_map<std::string, std::filesystem::path>& manifest_paths() const;
	const std::vector<CharacterAnimationContentEntry>& character_animation_entries() const;

private:
	bool load_character_animation_content(
		const std::filesystem::path& manifest_path,
		const std::filesystem::path& layout_path
	);

private:
	std::vector<AssetDirectoryEntry> _directories;
	std::unordered_map<std::string, std::filesystem::path> _manifest_paths;
	std::vector<CharacterAnimationContentEntry> _character_animation_entries;
	bool _has_assets_structure = false;
};
