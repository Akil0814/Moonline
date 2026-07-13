#include "../tools/logger.h"
#include "config_load_pipeline.h"

#include "../io/loaders/animation_manifest_loader.h"
#include "../io/loaders/audio_manifest_loader.h"
#include "../io/loaders/content_registry_loader.h"
#include "../io/loaders/effect_manifest_loader.h"
#include "../io/loaders/fonts_manifest_loader.h"
#include "../io/loaders/texture_manifest_loader.h"
#include "content_module_registry.h"

namespace elysia::loading
{
bool ConfigLoadPipeline::load(
	const std::filesystem::path& assets_structure_path,
	ConfigLoadResult& result
)
{
	result = ConfigLoadResult{};
	_error_message.clear();

	elysia::io::ContentRegistry content_registry;
	elysia::io::ContentRegistryLoader content_registry_loader;
	if (!content_registry_loader.load(assets_structure_path, content_registry))
	{
		fail("Config load pipeline failed: content registry load failed.");
		return false;
	}
	const elysia::io::CoreManifestPaths& manifest_paths = content_registry.required;

	elysia::io::FontsManifestLoader fonts_manifest_loader;
	if (!fonts_manifest_loader.load(manifest_paths.fonts, result.font_manifest))
	{
		fail("Config load pipeline failed: fonts manifest load failed.");
		return false;
	}

	elysia::io::AudioManifestLoader audio_manifest_loader;
	if (!audio_manifest_loader.load(manifest_paths.audio, result.audio_manifest))
	{
		fail("Config load pipeline failed: audio manifest load failed.");
		return false;
	}

	elysia::io::TextureManifestLoader texture_manifest_loader;
	if (!texture_manifest_loader.load(manifest_paths.textures, result.texture_manifest))
	{
		fail("Config load pipeline failed: textures manifest load failed.");
		return false;
	}

	elysia::io::AnimationManifestLoader animation_manifest_loader;
	if (!animation_manifest_loader.load(manifest_paths.animations, result.animation_manifest))
	{
		fail("Config load pipeline failed: animations manifest load failed.");
		return false;
	}

	elysia::io::EffectManifestLoader effect_manifest_loader;
	if (!effect_manifest_loader.load(manifest_paths.effects, result.effect_manifest))
	{
		fail("Config load pipeline failed: effects manifest load failed.");
		return false;
	}


	ContentModuleRegistry content_module_registry;
	std::string module_error;
	if (!content_module_registry.load_additional_modules(content_registry, result, module_error))
	{
		fail(module_error);
		return false;
	}

	return true;
}

const std::string& ConfigLoadPipeline::error_message() const
{
	return _error_message;
}

void ConfigLoadPipeline::fail(std::string message)
{
	_error_message = std::move(message);
	ELYSIA_LOG_ERROR("resource",_error_message);
}

}
