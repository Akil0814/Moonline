#pragma once

#include <filesystem>
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
		const std::filesystem::path& module_manifest_path,
		ConfigLoadResult& result,
		std::string& error_message
	) const = 0;
};
}
