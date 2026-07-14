#include "enemies_content_module.h"

#include "animated_entity_content_loader.h"
#include "config_load_pipeline.h"

namespace elysia::loading
{
std::string_view EnemiesContentModule::name() const
{
	return "enemies";
}

bool EnemiesContentModule::load(
	const std::filesystem::path& module_manifest_path,
	ConfigLoadResult& result,
	std::string& error_message
) const
{
	AnimatedEntityContentLoader loader;
	elysia::io::AnimatedEntityContent content;
	if (!loader.load(module_manifest_path, content, error_message))
		return false;

	result.enemies = ConfigLoadResult::EnemiesContent{std::move(content)};
	return true;
}
}
