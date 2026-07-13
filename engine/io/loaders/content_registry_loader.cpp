#include "../../tools/logger.h"
#include "content_registry_loader.h"

#include "../path/path_manager.h"

#include <array>
#include <string>
#include <string_view>

namespace elysia::io
{
namespace
{
constexpr std::string_view manifests_key = "manifests";
constexpr std::string_view required_key = "required";
constexpr std::string_view additional_key = "additional";
constexpr std::array<std::string_view, 7> required_manifest_keys{
	"fonts", "audio", "i18n", "textures", "animations", "effects", "configs"
};

bool read_required_manifest_path(
	const json& required,
	std::string_view key,
	PathManager& path_manager,
	std::filesystem::path& out_path
)
{
	const std::string key_string(key);
	if (!required.contains(key_string) || !required.at(key_string).is_string())
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: required manifest path is missing or invalid: " << key);
		return false;
	}

	out_path = path_manager.to_asset_path(required.at(key_string).get<std::string>());
	if (!std::filesystem::is_regular_file(out_path))
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: required manifest file does not exist: " << out_path);
		return false;
	}

	return true;
}

bool contains_required_key(std::string_view key)
{
	for (const std::string_view known_key : required_manifest_keys)
	{
		if (known_key == key)
			return true;
	}
	return false;
}
}

bool ContentRegistryLoader::load(
	const std::filesystem::path& content_registry_path,
	ContentRegistry& content_registry
) const
{
	content_registry = ContentRegistry{};

	JsonLoader loader;
	const JsonReadResult result = loader.open_file(content_registry_path);
	if (!result || !loader.root().is_object())
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: invalid JSON root: " << content_registry_path);
		return false;
	}

	if (loader.root().size() != 1 || !loader.root().contains(std::string(manifests_key)))
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: root must contain only manifests.");
		return false;
	}

	const json& manifests = loader.root().at(std::string(manifests_key));
	if (!manifests.is_object() || !manifests.contains(std::string(required_key)))
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: manifests.required is missing.");
		return false;
	}

	for (json::const_iterator item = manifests.begin(); item != manifests.end(); ++item)
	{
		if (item.key() != required_key && item.key() != additional_key)
		{
			ELYSIA_LOG_WARN("io", "Load content registry failed: unknown manifests key: " << item.key());
			return false;
		}
	}

	const json& required = manifests.at(std::string(required_key));
	if (!required.is_object())
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: manifests.required is not an object.");
		return false;
	}
	for (json::const_iterator item = required.begin(); item != required.end(); ++item)
	{
		if (!contains_required_key(item.key()))
		{
			ELYSIA_LOG_WARN("io", "Load content registry failed: unknown required manifest key: " << item.key());
			return false;
		}
	}

	PathManager* path_manager = PathManager::instance();
	if (!path_manager)
		return false;

	CoreManifestPaths& paths = content_registry.required;
	if (!read_required_manifest_path(required, "fonts", *path_manager, paths.fonts)
		|| !read_required_manifest_path(required, "audio", *path_manager, paths.audio)
		|| !read_required_manifest_path(required, "i18n", *path_manager, paths.i18n)
		|| !read_required_manifest_path(required, "textures", *path_manager, paths.textures)
		|| !read_required_manifest_path(required, "animations", *path_manager, paths.animations)
		|| !read_required_manifest_path(required, "effects", *path_manager, paths.effects)
		|| !read_required_manifest_path(required, "configs", *path_manager, paths.configs))
	{
		return false;
	}

	if (!manifests.contains(std::string(additional_key)))
		return true;

	const json& additional = manifests.at(std::string(additional_key));
	if (!additional.is_object())
	{
		ELYSIA_LOG_WARN("io", "Load content registry failed: manifests.additional is not an object.");
		return false;
	}

	for (json::const_iterator item = additional.begin(); item != additional.end(); ++item)
	{
		if (!item.value().is_object())
		{
			ELYSIA_LOG_WARN("io", "Load content registry failed: additional module config is not an object: " << item.key());
			return false;
		}
		content_registry.additional_modules.emplace(item.key(), item.value());
	}

	return true;
}
}
