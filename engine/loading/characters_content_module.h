#pragma once

#include "content_module.h"

namespace elysia::loading
{
class CharactersContentModule final : public ContentModule
{
public:
	std::string_view name() const override;
	bool load(
		const elysia::io::json& module_config,
		ConfigLoadResult& result,
		std::string& error_message
	) const override;
};
}
