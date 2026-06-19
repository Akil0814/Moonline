#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct AssetManifestPaths
{
	std::filesystem::path audio;

	std::filesystem::path fonts;

	std::filesystem::path i18n;

	std::filesystem::path map_textures;
	std::filesystem::path ui_textures;

	std::filesystem::path characters;
	std::filesystem::path character_animations;
	std::filesystem::path character_audio;
	std::filesystem::path character_effects;
	std::filesystem::path character_textures;
};

struct FontManifestEntry
{
	std::string key;
	std::filesystem::path file_path;
	int point_size = 0;
};

struct FontManifest
{
	std::vector<FontManifestEntry> fonts;
};

struct AudioManifestEntry
{
	std::string key;
	std::filesystem::path file_path;
};

struct AudioManifest
{
	std::vector<AudioManifestEntry> sounds;
	std::vector<AudioManifestEntry> music;
};

struct TextureManifestEntry
{
	std::string key;
	std::filesystem::path file_path;
};

struct TextureManifest
{
	std::vector<TextureManifestEntry> textures;
};

struct CharacterManifestEntry
{
	std::string id;
	std::string asset_key;
	std::filesystem::path config_path;
};

struct CharacterManifest
{
	std::vector<CharacterManifestEntry> characters;
};

struct CharacterConfig
{
	std::string id;
	std::string asset_key;
	std::filesystem::path texture_root;
	std::filesystem::path animation_config_path;
};

struct CharacterAnimationLayoutEntry
{
	std::filesystem::path path;
	std::filesystem::path segment_path;
	bool has_path = false;
	bool has_segment_path = false;
};

struct CharacterAnimationLayout
{
	std::unordered_map<std::string, CharacterAnimationLayoutEntry> animations;
};

struct CharacterEffectPlaybackConfig
{
	size_t frame_count = 0;
	double fps = 10.0;
	bool loop = true;
};

struct CharacterEffectLayoutEntry
{
	std::filesystem::path path;
	std::filesystem::path segment_path;
	bool has_path = false;
	bool has_segment_path = false;
	CharacterEffectPlaybackConfig playback;
	std::vector<CharacterEffectPlaybackConfig> segments;
};

struct CharacterEffectLayout
{
	std::unordered_map<std::string, CharacterEffectLayoutEntry> effects;
};

struct CharacterTextureLayoutEntry
{
	std::string key;
	std::filesystem::path path;
};

struct CharacterTextureLayout
{
	std::vector<CharacterTextureLayoutEntry> textures;
};

struct AnimationClipConfig
{
	std::string animation_name;
	std::filesystem::path path;
	size_t frame_count = 0;
	double fps = 10.0;
	bool loop = true;
	bool is_segment = false;
	size_t segment_index = 0;
};

struct AnimationConfig
{
	std::vector<AnimationClipConfig> clips;
};

struct CharacterAnimationContentEntry
{
	CharacterConfig character_config;
	AnimationConfig animation_config;
};

