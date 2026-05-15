#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct AssetDirectoryEntry
{
	std::string _group_name;
	std::filesystem::path _directory_path;
};

struct AssetsStructureManifest
{
	std::vector<AssetDirectoryEntry> _directories;
	std::unordered_map<std::string, std::filesystem::path> _manifest_paths;
};

struct CharacterManifestEntry
{
	std::string _id;
	std::string _asset_key;
	std::filesystem::path _config_path;
};

struct CharacterManifest
{
	std::vector<CharacterManifestEntry> _characters;
};

struct CharacterConfig
{
	std::string _id;
	std::string _asset_key;
	std::filesystem::path _texture_root;
	std::filesystem::path _animation_config_path;
};

struct AnimationClipConfig
{
	std::string _animation_name;
	std::filesystem::path _path;
	size_t _frame_count = 0;
	double _fps = 10.0;
	bool _loop = true;
	bool _is_segment = false;
	size_t _segment_index = 0;
};

struct AnimationConfig
{
	std::vector<AnimationClipConfig> _clips;
};
