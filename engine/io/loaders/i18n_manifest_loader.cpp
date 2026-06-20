#include "i18n_manifest_loader.h"

#include "../json/json_loader.h"

#include <iostream>
#include <utility>

namespace elysia::io
{
bool I18nManifestLoader::load(
	const std::filesystem::path& manifest_path,
	I18nManifest& manifest
) const
{
	manifest = I18nManifest{};

	JsonLoader loader;
	const JsonReadResult open_result = loader.open_file(manifest_path);
	if (!open_result.success)
	{
		std::cout << "Load i18n manifest failed: " << open_result.error << std::endl;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load i18n manifest failed: root is not an object: "
			<< manifest_path << std::endl;
		return false;
	}

	I18nManifest parsed_manifest;
	if (!loader.get("default_language", parsed_manifest.default_language)
		|| parsed_manifest.default_language.empty())
	{
		std::cout << "Load i18n manifest failed: default_language is missing or invalid: "
			<< manifest_path << std::endl;
		return false;
	}

	if (!loader.get_array("languages", parsed_manifest.languages)
		|| parsed_manifest.languages.empty())
	{
		std::cout << "Load i18n manifest failed: languages is missing or invalid: "
			<< manifest_path << std::endl;
		return false;
	}

	if (!loader.root().contains("file") || !loader.root().at("file").is_array())
	{
		std::cout << "Load i18n manifest failed: file is missing or not an array: "
			<< manifest_path << std::endl;
		return false;
	}

	for (const json& file_node : loader.root().at("file"))
	{
		if (!file_node.is_string())
		{
			std::cout << "Load i18n manifest failed: file entry is not a string."
				<< std::endl;
			return false;
		}

		const std::string file_path = file_node.get<std::string>();
		if (file_path.empty())
		{
			std::cout << "Load i18n manifest failed: file entry is empty." << std::endl;
			return false;
		}

		parsed_manifest.files.emplace_back(file_path);
	}

	if (parsed_manifest.files.empty())
	{
		std::cout << "Load i18n manifest failed: file list is empty: "
			<< manifest_path << std::endl;
		return false;
	}

	manifest = std::move(parsed_manifest);
	return true;
}

}
