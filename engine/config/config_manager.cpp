#include "config_manager.h"

#include <utility>

void ConfigManager::clear()
{
	_font_manifest = FontManifest{};
	_audio_manifest = AudioManifest{};
}

void ConfigManager::set_font_manifest(FontManifest manifest)
{
	_font_manifest = std::move(manifest);
}

void ConfigManager::set_audio_manifest(AudioManifest manifest)
{
	_audio_manifest = std::move(manifest);
}

const FontManifest& ConfigManager::font_manifest() const
{
	return _font_manifest;
}

const AudioManifest& ConfigManager::audio_manifest() const
{
	return _audio_manifest;
}
