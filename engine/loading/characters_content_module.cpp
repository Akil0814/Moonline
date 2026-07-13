#include "characters_content_module.h"

#include "config_load_pipeline.h"
#include "../io/loaders/animation_config_loader.h"
#include "../io/loaders/character_animation_layout_loader.h"
#include "../io/loaders/character_audio_layout_loader.h"
#include "../io/loaders/character_config_loader.h"
#include "../io/loaders/character_effect_layout_loader.h"
#include "../io/loaders/character_manifest_loader.h"
#include "../io/loaders/character_texture_layout_loader.h"
#include "../io/path/path_manager.h"

#include <array>
#include <filesystem>
#include <utility>

namespace elysia::loading
{
namespace
{
constexpr std::array<std::string_view, 5> characters_config_keys{
	"manifest", "animations", "audio", "effects", "textures"
};

bool has_characters_config_key(std::string_view key)
{
	for (const std::string_view known_key : characters_config_keys)
	{
		if (key == known_key)
			return true;
	}
	return false;
}

bool read_module_path(
	const elysia::io::json& config,
	std::string_view key,
	std::filesystem::path& out_path,
	std::string& error_message
)
{
	const std::string key_string(key);
	if (!config.contains(key_string) || !config.at(key_string).is_string())
	{
		error_message = "Characters content module failed: missing or invalid " + key_string + " path.";
		return false;
	}

	out_path = elysia::io::PathManager::instance()->to_asset_path(config.at(key_string).get<std::string>());
	if (!std::filesystem::is_regular_file(out_path))
	{
		error_message = "Characters content module failed: manifest file does not exist: " + out_path.string();
		return false;
	}
	return true;
}
}

std::string_view CharactersContentModule::name() const
{
	return "characters";
}

bool CharactersContentModule::load(
	const elysia::io::json& module_config,
	ConfigLoadResult& result,
	std::string& error_message
) const
{
	if (!module_config.is_object())
	{
		error_message = "Characters content module failed: config is not an object.";
		return false;
	}
	for (elysia::io::json::const_iterator item = module_config.begin(); item != module_config.end(); ++item)
	{
		if (!has_characters_config_key(item.key()))
		{
			error_message = "Characters content module failed: unknown config key: " + item.key();
			return false;
		}
	}

	std::filesystem::path manifest_path;
	std::filesystem::path animation_layout_path;
	std::filesystem::path audio_layout_path;
	std::filesystem::path effect_layout_path;
	std::filesystem::path texture_layout_path;
	if (!read_module_path(module_config, "manifest", manifest_path, error_message)
		|| !read_module_path(module_config, "animations", animation_layout_path, error_message)
		|| !read_module_path(module_config, "audio", audio_layout_path, error_message)
		|| !read_module_path(module_config, "effects", effect_layout_path, error_message)
		|| !read_module_path(module_config, "textures", texture_layout_path, error_message))
	{
		return false;
	}

	ConfigLoadResult::CharactersContent content;
	elysia::io::CharacterAnimationLayout animation_layout;
	elysia::io::CharacterAnimationLayoutLoader animation_layout_loader;
	if (!animation_layout_loader.load(animation_layout_path, animation_layout))
	{
		error_message = "Characters content module failed: animation layout load failed.";
		return false;
	}

	elysia::io::CharacterEffectLayoutLoader effect_layout_loader;
	if (!effect_layout_loader.load(effect_layout_path, content.effect_layout))
	{
		error_message = "Characters content module failed: effect layout load failed.";
		return false;
	}

	elysia::io::CharacterTextureLayoutLoader texture_layout_loader;
	if (!texture_layout_loader.load(texture_layout_path, content.texture_layout))
	{
		error_message = "Characters content module failed: texture layout load failed.";
		return false;
	}

	elysia::io::CharacterAudioLayoutLoader audio_layout_loader;
	if (!audio_layout_loader.load(audio_layout_path, content.audio_layout))
	{
		error_message = "Characters content module failed: audio layout load failed.";
		return false;
	}

	elysia::io::CharacterManifest character_manifest;
	elysia::io::CharacterManifestLoader character_manifest_loader;
	if (!character_manifest_loader.load(manifest_path, character_manifest))
	{
		error_message = "Characters content module failed: character manifest load failed.";
		return false;
	}

	elysia::io::CharacterConfigLoader character_config_loader;
	elysia::io::AnimationConfigLoader animation_config_loader;
	for (const elysia::io::CharacterManifestEntry& character_entry : character_manifest.characters)
	{
		elysia::io::CharacterConfig character_config;
		if (!character_config_loader.load(character_entry, character_config))
		{
			error_message = "Characters content module failed: character config load failed.";
			return false;
		}

		elysia::io::AnimationConfig animation_config;
		if (!animation_config_loader.load(character_config.animation_config_path, animation_layout, animation_config))
		{
			error_message = "Characters content module failed: animation config load failed.";
			return false;
		}

		content.animation_entries.push_back(elysia::io::CharacterAnimationContentEntry{
			std::move(character_config), std::move(animation_config)
		});
	}

	result.characters = std::move(content);
	return true;
}
}
