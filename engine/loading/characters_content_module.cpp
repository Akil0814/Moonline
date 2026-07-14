#include "characters_content_module.h"

#include "config_load_pipeline.h"
#include "animated_entity_content_loader.h"

namespace elysia::loading
{
std::string_view CharactersContentModule::name() const
{
	return "characters";
}

bool CharactersContentModule::load(
	const std::filesystem::path& module_manifest_path,
	ConfigLoadResult& result,
	std::string& error_message
) const
{
	AnimatedEntityContentLoader loader;
	elysia::io::AnimatedEntityContent content;
	if (!loader.load(module_manifest_path, content, error_message))
		return false;
	result.characters = ConfigLoadResult::CharactersContent{std::move(content)};
	return true;
}
}
