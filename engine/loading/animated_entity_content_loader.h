#pragma once

#include "../io/loaders/asset_config_types.h"

#include <filesystem>
#include <string>

namespace elysia::loading
{
class AnimatedEntityContentLoader
{
public:
	bool load(
		const std::filesystem::path& module_manifest_path,
		elysia::io::AnimatedEntityContent& content,
		std::string& error_message
	) const;
};
}
