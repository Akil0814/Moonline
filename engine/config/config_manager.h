#pragma once

#include "../io/loaders/asset_config_types.h"
#include "../tools/singleton.h"

class ConfigManager : public Singleton<ConfigManager>
{
	friend Singleton<ConfigManager>;

public:
	ConfigManager() = default;
	~ConfigManager() = default;

	void clear();

	void set_font_manifest(FontManifest manifest);
	void set_audio_manifest(AudioManifest manifest);

	const FontManifest& font_manifest() const;
	const AudioManifest& audio_manifest() const;

private:
	FontManifest _font_manifest;
	AudioManifest _audio_manifest;
};
