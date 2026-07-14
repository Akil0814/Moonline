#pragma once

#include "../json/json_loader.h"

#include <cstddef>
#include <filesystem>
#include <optional>
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
	std::filesystem::path configs;

};

struct BootstrapPaths
{
	std::filesystem::path app_config;
	std::filesystem::path preload_manifest;
};

struct ContentRegistry
{
	BootstrapPaths bootstrap;
	CoreManifestPaths required;
std::unordered_map<std::string, std::filesystem::path> additional_module_manifests;
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

struct AnimationEffectManifestEntry
{
	std::string key;
	std::string animation_key;
	float default_width = 0.0f;
	float default_height = 0.0f;
	double default_angle_degrees = 0.0;
};

struct AnimationEffectManifest
{
	std::vector<AnimationEffectManifestEntry> effects;
};

struct EntityManifestEntry
{
	std::string id;
	std::string asset_key;
	std::string animation_layout;
};

struct EntityManifest
{
	std::vector<EntityManifestEntry> entities;
};

struct AnimatedEntityResourceConfig
{
	std::string id;
	std::string asset_key;
	std::filesystem::path texture_root;
	std::filesystem::path audio_root;
};

struct AnimationLayoutEntry
{
	std::filesystem::path path;
	std::filesystem::path segment_path;
	bool has_path = false;
	bool has_segment_path = false;
};

struct AnimationLayout
{
	std::unordered_map<std::string, AnimationLayoutEntry> animations;
};

struct AnimationEffectPlaybackConfig
{
	size_t frame_count = 0;
	double fps = 10.0;
	bool loop = true;
};

struct AnimationEffectLayoutEntry
{
	std::filesystem::path path;
	std::filesystem::path segment_path;
	bool has_path = false;
	bool has_segment_path = false;
	float default_width = 0.0f;
	float default_height = 0.0f;
	double default_angle_degrees = 0.0;
	AnimationEffectPlaybackConfig playback;
	std::vector<AnimationEffectPlaybackConfig> segments;
};

struct AnimationEffectLayout
{
	std::unordered_map<std::string, AnimationEffectLayoutEntry> effects;
};

struct EntityTextureLayoutEntry
{
	std::string key;
	std::filesystem::path path;
};

struct EntityTextureLayout
{
	std::vector<EntityTextureLayoutEntry> textures;
};

struct EntityAudioLayoutEntry
{
	std::string key;
	std::filesystem::path path;
};

struct EntityAudioLayout
{
	std::vector<EntityAudioLayoutEntry> sounds;
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

struct AnimatedEntityAnimationContentEntry
{
	AnimatedEntityResourceConfig entity_config;
	AnimationConfig animation_config;
};

struct AnimatedEntityContent
{
	std::vector<AnimatedEntityResourceConfig> entities;
	std::vector<AnimatedEntityAnimationContentEntry> animation_entries;
	std::optional<EntityTextureLayout> texture_layout;
	std::optional<EntityAudioLayout> audio_layout;
	std::optional<AnimationEffectLayout> effect_layout;
};



}
