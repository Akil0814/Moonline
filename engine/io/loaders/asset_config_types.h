#pragma once

#include "../json/json_loader.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace elysia::io
{
struct CoreManifestPaths
{
	std::filesystem::path audio;

	std::filesystem::path fonts;

	std::filesystem::path i18n;

	std::filesystem::path textures;
	std::filesystem::path animations;
	std::filesystem::path effects;
	std::filesystem::path config_documents;

};

struct ContentRegistry
{
	CoreManifestPaths required;
	std::unordered_map<std::string, json> additional_modules;
};

struct FontManifestEntry
{
	std::string key;
	std::filesystem::path file_path;
};

struct FontManifest
{
	std::vector<int> point_sizes;
	std::vector<FontManifestEntry> fonts;
};

struct I18nManifest
{
	std::string default_language;
	std::vector<std::string> languages;
	std::vector<std::filesystem::path> files;
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

struct AnimationManifestEntry
{
	std::string key;
	std::filesystem::path directory_path;
	size_t frame_count = 0;
	double fps = 10.0;
	bool loop = true;
};

struct AnimationManifest
{
	std::vector<AnimationManifestEntry> animations;
};

struct EffectManifestEntry
{
	std::string key;
	std::string animation_key;
};

struct EffectManifest
{
	std::vector<EffectManifestEntry> effects;
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

struct CharacterAudioLayoutEntry
{
	std::string key;
	std::filesystem::path path;
};

struct CharacterAudioLayout
{
	std::vector<CharacterAudioLayoutEntry> sounds;
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



}
