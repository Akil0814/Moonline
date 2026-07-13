#pragma once

#include "../io/json/json_loader.h"

#include <string>
#include <string_view>

namespace elysia::loading
{
struct ConfigLoadResult;

class ContentModule
{
public:
	virtual ~ContentModule() = default;

	virtual std::string_view name() const = 0;
	virtual bool load(
		const elysia::io::json& module_config,
		ConfigLoadResult& result,
		std::string& error_message
	) const = 0;
};
}
