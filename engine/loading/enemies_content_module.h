#pragma once

#include "content_module.h"

namespace elysia::loading
{
class EnemiesContentModule final : public ContentModule
{
public:
	std::string_view name() const override;
	bool load(
		const std::filesystem::path& module_manifest_path,
		ConfigLoadResult& result,
		std::string& error_message
	) const override;
};
}
