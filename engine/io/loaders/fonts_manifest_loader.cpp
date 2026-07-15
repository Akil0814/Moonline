#include "../../tools/logger.h"
#include "fonts_manifest_loader.h"

#include "../json/json_duplicate_key_checker.h"
#include "../json/json_loader.h"
#include "../../resources/pipeline/resource_key_builder.h"
#include <array>
#include <utility>

namespace elysia::io
{
bool FontsManifestLoader::load(
	const std::filesystem::path& manifest_path,
	FontManifest& manifest
) const
{
	manifest = FontManifest{};
	if (has_duplicate_json_object_key(manifest_path))
	{
		ELYSIA_LOG_WARN("io", "Load fonts manifest failed: duplicate JSON object key: " << manifest_path);
		return false;
	}

	JsonLoader loader;
	JsonReadResult result = loader.open_file(manifest_path);
	if (!result)
	{
		ELYSIA_LOG_WARN("io","Load fonts manifest failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_WARN("io","Load fonts manifest failed: root is not an object: "
			<< manifest_path);
		return false;
	}

	if (!loader.root().contains("sizes") || !loader.root().at("sizes").is_array())
	{
		ELYSIA_LOG_WARN("io","Load fonts manifest failed: sizes is missing or not an array: "
			<< manifest_path);
		return false;
	}

	if (!loader.root().contains("fonts") || !loader.root().at("fonts").is_array())
	{
		ELYSIA_LOG_WARN("io","Load fonts manifest failed: fonts is missing or not an array: "
			<< manifest_path);
		return false;
	}

	FontManifest parsed_manifest;
	constexpr std::array<int,7> required_sizes{ 10,20,30,40,50,60,70 };
	const json& sizes = loader.root().at("sizes");
	if (sizes.size() != required_sizes.size())
	{
		ELYSIA_LOG_WARN("io","Load fonts manifest failed: sizes must contain the complete 10-70 step-10 scale.");
		return false;
	}

	for (std::size_t index = 0; index < required_sizes.size(); ++index)
	{
		if (!sizes.at(index).is_number_integer() || sizes.at(index).get<int>() != required_sizes[index])
		{
			ELYSIA_LOG_WARN("io","Load fonts manifest failed: sizes must equal 10,20,30,40,50,60,70.");
			return false;
		}
		parsed_manifest.point_sizes.push_back(required_sizes[index]);
	}

	const json& fonts = loader.root().at("fonts");
	size_t font_index = 0;
	for (const json& font : fonts)
	{
		const size_t current_index = font_index++;
		if (!font.is_object())
		{
			ELYSIA_LOG_WARN("io","Load fonts manifest failed: font entry is not an object.");
			return false;
		}

		if (!font.contains("key") || !font.at("key").is_string())
		{
			ELYSIA_LOG_WARN("io","Load fonts manifest failed: key is missing or not a string.");
			return false;
		}

		if (!font.contains("file") || !font.at("file").is_string())
		{
			ELYSIA_LOG_WARN("io","Load fonts manifest failed: file is missing or not a string.");
			return false;
		}

		FontManifestEntry entry;
		entry.key = font.at("key").get<std::string>();
		entry.file_path = font.at("file").get<std::string>();
		if (entry.key.empty() || entry.file_path.empty())
		{
			ELYSIA_LOG_WARN("io","Load fonts manifest failed: font key and file must not be empty.");
			return false;
		}
		for (auto field = font.begin(); field != font.end(); ++field)
			if (field.key() != "key" && field.key() != "file") return false;
		std::string key_error;
		if (!elysia::resources::ResourceKeyBuilder::validate_key(entry.key, key_error))
		{
			ELYSIA_LOG_WARN("io", "Load fonts manifest failed: " << key_error);
			return false;
		}
		entry.origin = elysia::resources::make_resource_origin(
			manifest_path, "/fonts/" + std::to_string(current_index), {}, "fonts", {}, entry.key);
		parsed_manifest.fonts.push_back(std::move(entry));
	}
	if (parsed_manifest.fonts.empty())
	{
		ELYSIA_LOG_WARN("io","Load fonts manifest failed: fonts must not be empty.");
		return false;
	}

	manifest = std::move(parsed_manifest);
	return true;
}

}
