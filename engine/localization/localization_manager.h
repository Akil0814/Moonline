#pragma once

#include "localized_text_style.h"
#include "text_texture_cache.h"
#include "../io/loaders/asset_config_types.h"
#include "../tools/singleton.h"

#include <SDL.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class LocalizationManager : public Singleton<LocalizationManager>
{
	friend Singleton<LocalizationManager>;

public:
	bool init(
		SDL_Renderer* renderer,
		const std::filesystem::path& manifest_path,
		std::string initial_language
	);
	void shutdown();

	std::string_view tr(std::string_view key) const;
	SDL_Texture* get_text_texture(std::string_view key, const LocalizedTextStyle& style);

	bool set_language(std::string language);
	const std::string& current_language() const;
	const std::vector<std::string>& supported_languages() const;
	void clear_texture_cache();

private:
	using TranslationTable = std::unordered_map<std::string, std::string>;

	bool is_supported_language(const std::string& language) const;
	bool ensure_language_loaded(const std::string& language);
	bool load_language_table(const std::string& language, TranslationTable& out_table) const;
	std::filesystem::path resolve_locale_directory(const std::string& language) const;
	std::string_view lookup_translation(
		const TranslationTable& table,
		std::string_view key
	) const;
	std::string map_font_key(const std::string& language, int point_size) const;
	CachedTexturePtr create_text_texture(
		std::string_view key,
		const LocalizedTextStyle& style
	);

private:
	SDL_Renderer* _renderer = nullptr;
	std::filesystem::path _manifest_path;
	std::filesystem::path _i18n_root;
	I18nManifest _manifest;
	TextTextureCache _text_texture_cache;
	std::unordered_map<std::string, TranslationTable> _translation_tables;
	std::string _current_language;
	bool _initialized = false;
};
