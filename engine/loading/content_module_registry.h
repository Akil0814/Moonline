#pragma once

#include "../io/loaders/asset_config_types.h"

#include <string>

namespace elysia::loading
{
struct ConfigLoadResult;

class ContentModuleRegistry
{
public:
	bool load_additional_modules(
		const elysia::io::ContentRegistry& content_registry,
		ConfigLoadResult& result,
		std::string& error_message
	) const;
};
}
