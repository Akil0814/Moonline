#include "fonts_manifest_loader.h"

#include "../json/json_loader.h"

#include <iostream>
#include <array>
#include <unordered_set>
#include <utility>

namespace elysia::io
{
bool FontsManifestLoader::load(
	const std::filesystem::path& manifest_path,
	FontManifest& manifest
) const
{
	manifest = FontManifest{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(manifest_path);
	if (!result)
	{
		std::cout << "Load fonts manifest failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load fonts manifest failed: root is not an object: "
			<< manifest_path << std::endl;
		return false;
	}

	if (!loader.root().contains("sizes") || !loader.root().at("sizes").is_array())
	{
		std::cout << "Load fonts manifest failed: sizes is missing or not an array: "
			<< manifest_path << std::endl;
		return false;
	}

	if (!loader.root().contains("fonts") || !loader.root().at("fonts").is_array())
	{
		std::cout << "Load fonts manifest failed: fonts is missing or not an array: "
			<< manifest_path << std::endl;
		return false;
	}

	FontManifest parsed_manifest;
	constexpr std::array<int,7> required_sizes{ 10,20,30,40,50,60,70 };
	const json& sizes = loader.root().at("sizes");
	if (sizes.size() != required_sizes.size())
	{
		std::cout << "Load fonts manifest failed: sizes must contain the complete 10-70 step-10 scale."
			<< std::endl;
		return false;
	}

	for (std::size_t index = 0; index < required_sizes.size(); ++index)
	{
		if (!sizes.at(index).is_number_integer() || sizes.at(index).get<int>() != required_sizes[index])
		{
			std::cout << "Load fonts manifest failed: sizes must equal 10,20,30,40,50,60,70."
				<< std::endl;
			return false;
		}
		parsed_manifest.point_sizes.push_back(required_sizes[index]);
	}

	const json& fonts = loader.root().at("fonts");
	std::unordered_set<std::string> font_keys;
	for (const json& font : fonts)
	{
		if (!font.is_object())
		{
			std::cout << "Load fonts manifest failed: font entry is not an object."
				<< std::endl;
			return false;
		}

		if (!font.contains("key") || !font.at("key").is_string())
		{
			std::cout << "Load fonts manifest failed: key is missing or not a string."
				<< std::endl;
			return false;
		}

		if (!font.contains("file") || !font.at("file").is_string())
		{
			std::cout << "Load fonts manifest failed: file is missing or not a string."
				<< std::endl;
			return false;
		}

		FontManifestEntry entry;
		entry.key = font.at("key").get<std::string>();
		entry.file_path = font.at("file").get<std::string>();
		if (entry.key.empty() || entry.file_path.empty())
		{
			std::cout << "Load fonts manifest failed: font key and file must not be empty."
				<< std::endl;
			return false;
		}
		if (!font_keys.insert(entry.key).second)
		{
			std::cout << "Load fonts manifest failed: duplicate font key: " << entry.key << std::endl;
			return false;
		}
		parsed_manifest.fonts.push_back(std::move(entry));
	}
	if (parsed_manifest.fonts.empty())
	{
		std::cout << "Load fonts manifest failed: fonts must not be empty." << std::endl;
		return false;
	}

	manifest = std::move(parsed_manifest);
	return true;
}

}
