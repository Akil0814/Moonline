#include "content_module_registry.h"

#include "characters_content_module.h"
#include "content_module.h"

#include <array>
#include <string>

namespace elysia::loading
{
bool ContentModuleRegistry::load_additional_modules(
	const elysia::io::ContentRegistry& content_registry,
	ConfigLoadResult& result,
	std::string& error_message
) const
{
	static const CharactersContentModule characters_module;
	const std::array<const ContentModule*, 1> modules{ &characters_module };

	for (const auto& [module_name, module_config] : content_registry.additional_modules)
	{
		bool is_registered = false;
		for (const ContentModule* module : modules)
		{
			if (module->name() == module_name)
			{
				is_registered = true;
				break;
			}
		}

		if (!is_registered)
		{
			error_message = "Config load pipeline failed: unknown additional content module: " + module_name;
			return false;
		}
	}

	for (const ContentModule* module : modules)
	{
		const auto config = content_registry.additional_modules.find(std::string(module->name()));
		if (config == content_registry.additional_modules.end())
			continue;

		if (!module->load(config->second, result, error_message))
			return false;
	}

	return true;
}
}
