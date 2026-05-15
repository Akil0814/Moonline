#include "file_manager.h"

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace
{
	constexpr std::string_view directory_suffix = "_directories";

	bool path_starts_with_assets(const std::filesystem::path& path)
	{
		std::filesystem::path::iterator iterator = path.begin();
		if (iterator == path.end())
			return false;

		return iterator->string() == "assets";
	}
}

bool FileManager::load_assets_strcutre(const std::filesystem::path& assets_structure_path)
{
	_directories.clear();
	_manifest_paths.clear();
	_atlas_load_requests.clear();
	_animation_build_requests.clear();
	_find_all_folder = false;
	_assets_root.clear();
	_project_root.clear();

	JsonLoader loader;
	JsonReadResult result = loader.open_file(assets_structure_path);
	if (result.success == false)
	{
		std::cout << "Load assets structure failed: " << result.error;
		return false;
	}

	std::vector<std::pair<std::string, std::string>> directories;
	std::unordered_map<std::string, std::filesystem::path> manifest_paths;
	_assets_root = assets_structure_path.parent_path();
	_project_root = _assets_root.parent_path();

	try
	{
		if (!loader.root().is_object())
		{
			std::cout << "Load assets structure failed: JSON root is not an object: "
				<< assets_structure_path << std::endl;
			return false;
		}

		for (json::const_iterator item = loader.root().begin();
			item != loader.root().end();
			++item)
		{
			std::string item_key = item.key();
			const json& item_value = item.value();

			if (item_key.ends_with(directory_suffix))
			{
				if (!item_value.is_array())
				{
					std::cout << "Load assets structure failed: key is not an array: "
						<< item_key << std::endl;
					return false;
				}

				std::string group_name = item_key;
				group_name.erase(group_name.size() - directory_suffix.size());

				if (group_name.empty())
				{
					std::cout << "Load assets structure failed: empty directory group from key: "
						<< item_key << std::endl;
					return false;
				}

				for (const json& directory : item_value)
				{
					if (!directory.is_string())
					{
						std::cout << "Load assets structure failed: directory entry is not a string in key: "
							<< item_key << std::endl;
						return false;
					}

					const std::string directory_name = directory.get<std::string>();
					if (directory_name.empty())
					{
						std::cout << "Load assets structure failed: empty directory entry in key: "
							<< item_key << std::endl;
						return false;
					}

					std::filesystem::path directory_path = directory_name;
					if (!directory_path.is_absolute())
					{
						if (directory_path.has_parent_path())
							directory_path = _assets_root / directory_path;
						else
							directory_path = _assets_root / group_name / directory_path;
					}

					directory_path = directory_path.lexically_normal();

					if (!std::filesystem::is_directory(directory_path))
					{
						std::cout << "Load assets structure failed: directory does not exist: "
							<< directory_path << std::endl;
						return false;
					}

					directories.emplace_back(group_name, directory_path.string());
				}

				continue;
			}

			if (item_key == "manifests")
			{
				if (!item_value.is_object())
				{
					std::cout << "Load assets structure failed: manifests is not an object."
						<< std::endl;
					return false;
				}

				for (json::const_iterator manifest = item_value.begin();
					manifest != item_value.end();
					++manifest)
				{
					if (!manifest.value().is_string())
					{
						std::cout << "Load assets structure failed: manifest path is not a string: "
							<< manifest.key() << std::endl;
						return false;
					}

					std::filesystem::path manifest_path =
						resolve_asset_path(manifest.value().get<std::string>());
					if (!std::filesystem::is_regular_file(manifest_path))
					{
						std::cout << "Load assets structure failed: manifest file does not exist: "
							<< manifest_path << std::endl;
						return false;
					}

					manifest_paths.emplace(manifest.key(), manifest_path);
				}

				continue;
			}

			std::cout << "Load assets structure failed: unknown root key: "
				<< item_key << std::endl;
			return false;
		}
	}
	catch (const std::filesystem::filesystem_error&)
	{
		std::cout << "Load assets structure failed: filesystem error while reading: "
			<< assets_structure_path << std::endl;
		return false;
	}

	_directories = std::move(directories);
	_manifest_paths = std::move(manifest_paths);
	_find_all_folder = true;

	return true;
}

bool FileManager::load_configs()
{
	_atlas_load_requests.clear();
	_animation_build_requests.clear();

	if (!_find_all_folder)
	{
		std::cout << "Load configs failed: assets structure is not loaded." << std::endl;
		return false;
	}

	std::unordered_map<std::string, std::filesystem::path>::const_iterator iterator =
		_manifest_paths.find("characters");
	if (iterator == _manifest_paths.end())
	{
		std::cout << "Load configs failed: characters manifest is missing." << std::endl;
		return false;
	}

	return load_character_manifest(iterator->second);
}

bool FileManager::load_textures()
{
	return true;
}

bool FileManager::load_audio()
{
	return true;
}

bool FileManager::load_i18n()
{
	return true;
}

const std::vector<AtlasLoadRequest>& FileManager::atlas_load_requests() const
{
	return _atlas_load_requests;
}

const std::vector<AnimationBuildRequest>& FileManager::animation_build_requests() const
{
	return _animation_build_requests;
}

bool FileManager::load_character_manifest(const std::filesystem::path& manifest_path)
{
	JsonLoader loader;
	JsonReadResult result = loader.open_file(manifest_path);
	if (!result)
	{
		std::cout << "Load character manifest failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character manifest failed: root is not an object: "
			<< manifest_path << std::endl;
		return false;
	}

	if (!loader.root().contains("characters") || !loader.root().at("characters").is_array())
	{
		std::cout << "Load character manifest failed: characters is missing or not an array: "
			<< manifest_path << std::endl;
		return false;
	}

	const json& characters = loader.root().at("characters");
	for (const json& character : characters)
	{
		if (!character.is_object())
		{
			std::cout << "Load character manifest failed: character entry is not an object."
				<< std::endl;
			return false;
		}

		bool enabled = true;
		if (character.contains("enabled"))
		{
			if (!character.at("enabled").is_boolean())
			{
				std::cout << "Load character manifest failed: enabled is not a bool."
					<< std::endl;
				return false;
			}
			enabled = character.at("enabled").get<bool>();
		}

		if (!enabled)
			continue;

		if (!character.contains("id") || !character.at("id").is_string())
		{
			std::cout << "Load character manifest failed: id is missing or not a string."
				<< std::endl;
			return false;
		}

		if (!character.contains("asset_key") || !character.at("asset_key").is_string())
		{
			std::cout << "Load character manifest failed: asset_key is missing or not a string."
				<< std::endl;
			return false;
		}

		if (!character.contains("config") || !character.at("config").is_string())
		{
			std::cout << "Load character manifest failed: config is missing or not a string."
				<< std::endl;
			return false;
		}

		std::filesystem::path config_path =
			resolve_asset_path(character.at("config").get<std::string>());
		if (!std::filesystem::is_regular_file(config_path))
		{
			std::cout << "Load character manifest failed: config file does not exist: "
				<< config_path << std::endl;
			return false;
		}

		if (!load_character_config(
			character.at("id").get<std::string>(),
			character.at("asset_key").get<std::string>(),
			config_path))
		{
			return false;
		}
	}

	return true;
}

bool FileManager::load_character_config(
	const std::string& manifest_id,
	const std::string& manifest_asset_key,
	const std::filesystem::path& config_path
)
{
	JsonLoader loader;
	JsonReadResult result = loader.open_file(config_path);
	if (!result)
	{
		std::cout << "Load character config failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character config failed: root is not an object: "
			<< config_path << std::endl;
		return false;
	}

	std::string character_id = manifest_id;
	if (loader.root().contains("id") && loader.root().at("id").is_string())
		character_id = loader.root().at("id").get<std::string>();

	std::string asset_key = manifest_asset_key;
	if (loader.root().contains("asset_key") && loader.root().at("asset_key").is_string())
		asset_key = loader.root().at("asset_key").get<std::string>();

	if (character_id.empty() || asset_key.empty())
	{
		std::cout << "Load character config failed: character id or asset key is empty: "
			<< config_path << std::endl;
		return false;
	}

	if (!loader.root().contains("resources") || !loader.root().at("resources").is_object())
	{
		std::cout << "Load character config failed: resources is missing or not an object: "
			<< config_path << std::endl;
		return false;
	}

	const json& resources = loader.root().at("resources");
	if (!resources.contains("texture_root") || !resources.at("texture_root").is_string())
	{
		std::cout << "Load character config failed: texture_root is missing or not a string: "
			<< config_path << std::endl;
		return false;
	}

	if (!resources.contains("animation_config") || !resources.at("animation_config").is_string())
	{
		std::cout << "Load character config failed: animation_config is missing or not a string: "
			<< config_path << std::endl;
		return false;
	}

	std::filesystem::path texture_root =
		resolve_asset_path(resources.at("texture_root").get<std::string>());
	if (!std::filesystem::is_directory(texture_root))
	{
		std::cout << "Load character config failed: texture root does not exist: "
			<< texture_root << std::endl;
		return false;
	}

	std::filesystem::path animation_config_path =
		resolve_asset_path(resources.at("animation_config").get<std::string>());
	if (!std::filesystem::is_regular_file(animation_config_path))
	{
		std::cout << "Load character config failed: animation config does not exist: "
			<< animation_config_path << std::endl;
		return false;
	}

	return load_animation_config(character_id, texture_root, animation_config_path);
}

bool FileManager::load_animation_config(
	const std::string& character_id,
	const std::filesystem::path& texture_root,
	const std::filesystem::path& animation_config_path
)
{
	JsonLoader loader;
	JsonReadResult result = loader.open_file(animation_config_path);
	if (!result)
	{
		std::cout << "Load animation config failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load animation config failed: root is not an object: "
			<< animation_config_path << std::endl;
		return false;
	}

	for (json::const_iterator animation = loader.root().begin();
		animation != loader.root().end();
		++animation)
	{
		if (!animation.value().is_object())
		{
			std::cout << "Load animation config failed: animation entry is not an object: "
				<< animation.key() << std::endl;
			return false;
		}

		const json& animation_node = animation.value();
		if (animation_node.contains("segments"))
		{
			if (!animation_node.at("segments").is_array())
			{
				std::cout << "Load animation config failed: segments is not an array: "
					<< animation.key() << std::endl;
				return false;
			}

			const json& segments = animation_node.at("segments");
			if (segments.empty())
			{
				std::cout << "Load animation config failed: segments is empty: "
					<< animation.key() << std::endl;
				return false;
			}

			for (size_t segment_index = 0; segment_index < segments.size(); ++segment_index)
			{
				if (!segments[segment_index].is_object())
				{
					std::cout << "Load animation config failed: segment is not an object: "
						<< animation.key() << std::endl;
					return false;
				}

				if (!append_animation_clip(
					character_id,
					animation.key(),
					true,
					segment_index,
					segments[segment_index],
					texture_root))
				{
					return false;
				}
			}

			continue;
		}

		if (!append_animation_clip(character_id, animation.key(), false, 0, animation_node, texture_root))
			return false;
	}

	return true;
}

bool FileManager::append_animation_clip(
	const std::string& character_id,
	const std::string& animation_name,
	bool is_segment,
	size_t segment_index,
	const json& clip_node,
	const std::filesystem::path& texture_root
)
{
	if (!clip_node.contains("path") || !clip_node.at("path").is_string())
	{
		std::cout << "Load animation clip failed: path is missing or not a string: "
			<< animation_name << std::endl;
		return false;
	}

	if (!clip_node.contains("frame_count") || !clip_node.at("frame_count").is_number_integer())
	{
		std::cout << "Load animation clip failed: frame_count is missing or invalid: "
			<< animation_name << std::endl;
		return false;
	}

	if (!clip_node.contains("fps") || !clip_node.at("fps").is_number())
	{
		std::cout << "Load animation clip failed: fps is missing or invalid: "
			<< animation_name << std::endl;
		return false;
	}

	if (!clip_node.contains("loop") || !clip_node.at("loop").is_boolean())
	{
		std::cout << "Load animation clip failed: loop is missing or invalid: "
			<< animation_name << std::endl;
		return false;
	}

	std::filesystem::path directory_path =
		(texture_root / clip_node.at("path").get<std::string>()).lexically_normal();
	if (!std::filesystem::is_directory(directory_path))
	{
		std::cout << "Load animation clip failed: animation directory does not exist: "
			<< directory_path << std::endl;
		return false;
	}

	const int frame_count_value = clip_node.at("frame_count").get<int>();
	const double fps = clip_node.at("fps").get<double>();
	if (frame_count_value <= 0 || fps <= 0.0)
	{
		std::cout << "Load animation clip failed: frame_count or fps is invalid: "
			<< animation_name << std::endl;
		return false;
	}
	const size_t frame_count = static_cast<size_t>(frame_count_value);

	std::string animation_key = character_id + "." + animation_name;
	if (is_segment)
		animation_key += "." + std::to_string(segment_index);

	AtlasLoadRequest atlas_request;
	atlas_request._atlas_key = animation_key;
	atlas_request._directory_path = directory_path;
	atlas_request._frame_count = frame_count;

	AnimationBuildRequest animation_request;
	animation_request._animation_key = animation_key;
	animation_request._atlas_key = atlas_request._atlas_key;
	animation_request._fps = fps;
	animation_request._loop = clip_node.at("loop").get<bool>();
	animation_request._segment_index = segment_index;

	_atlas_load_requests.push_back(std::move(atlas_request));
	_animation_build_requests.push_back(std::move(animation_request));

	return true;
}

std::filesystem::path FileManager::resolve_asset_path(
	const std::filesystem::path& path
) const
{
	if (path.is_absolute())
		return path.lexically_normal();

	if (path_starts_with_assets(path))
		return (_project_root / path).lexically_normal();

	return (_assets_root / path).lexically_normal();
}
