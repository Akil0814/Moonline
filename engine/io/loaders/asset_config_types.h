#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct AssetManifestPaths
{
	std::filesystem::path _characters;
	std::filesystem::path _character_animations;
	std::filesystem::path _character_audio;
	std::filesystem::path _character_effects;
	std::filesystem::path _character_textures;
	std::filesystem::path _audio;
	std::filesystem::path _fonts;
	std::filesystem::path _i18n;
	std::filesystem::path _map_textures;
	std::filesystem::path _preload;
	std::filesystem::path _ui_textures;
};

struct FontManifestEntry
{
	std::string _key;
	std::filesystem::path _file_path;
	int _point_size = 0;
};

struct FontManifest
{
	std::vector<FontManifestEntry> _fonts;
};

struct AudioManifestEntry
{
	std::string _key;
	std::filesystem::path _file_path;
};

struct AudioManifest
{
	std::vector<AudioManifestEntry> _sounds;
	std::vector<AudioManifestEntry> _music;
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

struct CharacterAnimationLayoutEntry
{
	std::filesystem::path _path;
	std::filesystem::path _segment_path;
	bool _has_path = false;
	bool _has_segment_path = false;
};

struct CharacterAnimationLayout
{
	std::unordered_map<std::string, CharacterAnimationLayoutEntry> _animations;
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

struct CharacterAnimationContentEntry
{
	CharacterConfig _character_config;
	AnimationConfig _animation_config;
};
