#include "config_manager.h"

#include "../io/loaders/animation_config_loader.h"
#include "../io/loaders/assets_structure_loader.h"
#include "../io/loaders/character_animation_layout_loader.h"
#include "../io/loaders/character_config_loader.h"
#include "../io/loaders/character_manifest_loader.h"

#include <iostream>
#include <utility>

bool ConfigManager::load_assets_structure(const std::filesystem::path& assets_structure_path)
{
	clear();

	AssetsStructureManifest manifest;
	AssetsStructureLoader loader;
	if (!loader.load(assets_structure_path, manifest))
		return false;

	_directories = std::move(manifest._directories);
	_manifest_paths = std::move(manifest._manifest_paths);
	_has_assets_structure = true;
	return true;
}

bool ConfigManager::load_character_animation_content()
{
	_character_animation_entries.clear();

	if (!_has_assets_structure)
	{
		std::cout << "Load character animation content failed: assets structure is not loaded."
			<< std::endl;
		return false;
	}

	const std::unordered_map<std::string, std::filesystem::path>::const_iterator manifest_iterator =
		_manifest_paths.find("characters");
	if (manifest_iterator == _manifest_paths.end())
	{
		std::cout << "Load character animation content failed: characters manifest is missing."
			<< std::endl;
		return false;
	}

	const std::unordered_map<std::string, std::filesystem::path>::const_iterator layout_iterator =
		_manifest_paths.find("character_animations");
	if (layout_iterator == _manifest_paths.end())
	{
		std::cout << "Load character animation content failed: character animation layout is missing."
			<< std::endl;
		return false;
	}

	return load_character_animation_content(
		manifest_iterator->second,
		layout_iterator->second
	);
}

void ConfigManager::clear()
{
	_directories.clear();
	_manifest_paths.clear();
	_character_animation_entries.clear();
	_has_assets_structure = false;
}

const std::vector<AssetDirectoryEntry>& ConfigManager::directories() const
{
	return _directories;
}

const std::unordered_map<std::string, std::filesystem::path>& ConfigManager::manifest_paths() const
{
	return _manifest_paths;
}

const std::vector<CharacterAnimationContentEntry>& ConfigManager::character_animation_entries() const
{
	return _character_animation_entries;
}

bool ConfigManager::load_character_animation_content(
	const std::filesystem::path& manifest_path,
	const std::filesystem::path& layout_path
)
{
	CharacterAnimationLayout character_animation_layout;
	CharacterAnimationLayoutLoader character_animation_layout_loader;
	if (!character_animation_layout_loader.load(layout_path, character_animation_layout))
		return false;

	CharacterManifest character_manifest;
	CharacterManifestLoader character_manifest_loader;
	if (!character_manifest_loader.load(manifest_path, character_manifest))
		return false;

	CharacterConfigLoader character_config_loader;
	AnimationConfigLoader animation_config_loader;

	for (const CharacterManifestEntry& character_entry : character_manifest._characters)
	{
		CharacterConfig character_config;
		if (!character_config_loader.load(character_entry, character_config))
			return false;

		AnimationConfig animation_config;
		if (!animation_config_loader.load(
			character_config._animation_config_path,
			character_animation_layout,
			animation_config))
		{
			return false;
		}

		_character_animation_entries.push_back(CharacterAnimationContentEntry{
			std::move(character_config),
			std::move(animation_config)
			});
	}

	return true;
}
