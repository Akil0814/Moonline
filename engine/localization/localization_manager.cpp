#include "localization_manager.h"

#include "../io/json/json_loader.h"
#include "../io/loaders/i18n_manifest_loader.h"
#include "../io/path/path_manager.h"
#include "../resources/resource_manager.h"

#include <SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <utility>

namespace
{
using TranslationTable = std::unordered_map<std::string, std::string>;

bool flatten_locale_json(
	const json& node,
	const std::string& prefix,
	TranslationTable& out_table
)
{
	if (node.is_string())
	{
		out_table[prefix] = node.get<std::string>();
		return true;
	}

	if (!node.is_object())
		return false;

	for (auto iterator = node.begin(); iterator != node.end(); ++iterator)
	{
		const std::string key = prefix.empty()
			? iterator.key()
			: prefix + "." + iterator.key();
		if (!flatten_locale_json(iterator.value(), key, out_table))
			return false;
	}

	return true;
}

std::string replace_underscores_with_hyphens(std::string value)
{
	std::replace(value.begin(), value.end(), '_', '-');
	return value;
}
}

bool LocalizationManager::init(
	SDL_Renderer* renderer,
	const std::filesystem::path& manifest_path,
	std::string initial_language
)
{
	shutdown();

	if (!renderer)
	{
		std::cout << "Localization init failed: renderer is null." << std::endl;
		return false;
	}

	I18nManifestLoader manifest_loader;
	if (!manifest_loader.load(manifest_path, _manifest))
	{
		std::cout << "Localization init failed: i18n manifest load failed: "
			<< manifest_path << std::endl;
		return false;
	}

	if (_manifest.default_language.empty())
	{
		std::cout << "Localization init failed: default language is empty." << std::endl;
		return false;
	}

	if (_manifest.languages.empty())
	{
		std::cout << "Localization init failed: supported language list is empty."
			<< std::endl;
		return false;
	}

	if (!is_supported_language(_manifest.default_language))
		_manifest.languages.push_back(_manifest.default_language);

	PathManager* path_manager = PathManager::instance();
	if (!path_manager->is_initialized())
	{
		std::cout << "Localization init failed: path manager is not initialized."
			<< std::endl;
		return false;
	}

	_renderer = renderer;
	_manifest_path = manifest_path;
	_i18n_root = path_manager->assets() / "i18n";

	if (!ensure_language_loaded(_manifest.default_language))
	{
		shutdown();
		return false;
	}

	if (initial_language.empty() || !is_supported_language(initial_language))
		initial_language = _manifest.default_language;

	if (!ensure_language_loaded(initial_language))
		initial_language = _manifest.default_language;

	_current_language = initial_language;
	_initialized = true;
	_text_texture_cache.clear();
	return true;
}

void LocalizationManager::shutdown()
{
	_text_texture_cache.clear();
	_translation_tables.clear();
	_manifest = I18nManifest{};
	_manifest_path.clear();
	_i18n_root.clear();
	_current_language.clear();
	_renderer = nullptr;
	_initialized = false;
}

std::string_view LocalizationManager::tr(std::string_view key) const
{
	if (!_initialized)
		return key;

	const auto current_table_iterator = _translation_tables.find(_current_language);
	if (current_table_iterator != _translation_tables.end())
	{
		const std::string_view current_translation =
			lookup_translation(current_table_iterator->second, key);
		if (current_translation != key)
			return current_translation;
	}

	const auto default_table_iterator = _translation_tables.find(_manifest.default_language);
	if (default_table_iterator != _translation_tables.end())
	{
		const std::string_view default_translation =
			lookup_translation(default_table_iterator->second, key);
		if (default_translation != key)
			return default_translation;
	}

	return key;
}

SDL_Texture* LocalizationManager::get_text_texture(
	std::string_view key,
	const LocalizedTextStyle& style
)
{
	if (!_initialized)
		return nullptr;

	return _text_texture_cache.get_or_create(
		_current_language,
		key,
		style,
		[this, key, style]()
		{
			return create_text_texture(key, style);
		});
}

bool LocalizationManager::set_language(std::string language)
{
	if (!_initialized)
	{
		std::cout << "Set language failed: localization manager is not initialized."
			<< std::endl;
		return false;
	}

	if (!is_supported_language(language))
	{
		std::cout << "Set language failed: unsupported language: "
			<< language << std::endl;
		return false;
	}

	if (!ensure_language_loaded(language))
		return false;

	_current_language = std::move(language);
	_text_texture_cache.clear();
	return true;
}

const std::string& LocalizationManager::current_language() const
{
	return _current_language;
}

const std::vector<std::string>& LocalizationManager::supported_languages() const
{
	return _manifest.languages;
}

void LocalizationManager::clear_texture_cache()
{
	_text_texture_cache.clear();
}

bool LocalizationManager::is_supported_language(const std::string& language) const
{
	return std::find(
		_manifest.languages.begin(),
		_manifest.languages.end(),
		language) != _manifest.languages.end();
}

bool LocalizationManager::ensure_language_loaded(const std::string& language)
{
	if (_translation_tables.contains(language))
		return true;

	TranslationTable table;
	if (!load_language_table(language, table))
		return false;

	_translation_tables.emplace(language, std::move(table));
	return true;
}

bool LocalizationManager::load_language_table(
	const std::string& language,
	TranslationTable& out_table
) const
{
	const std::filesystem::path locale_directory = resolve_locale_directory(language);
	if (locale_directory.empty())
	{
		std::cout << "Load language table failed: locale directory not found for "
			<< language << std::endl;
		return false;
	}

	TranslationTable merged_table;
	for (const std::filesystem::path& relative_file_path : _manifest.files)
	{
		const std::filesystem::path full_file_path = locale_directory / relative_file_path;
		JsonLoader loader;
		const JsonReadResult open_result = loader.open_file(full_file_path);
		if (!open_result.success)
		{
			std::cout << "Load language table failed: " << open_result.error << std::endl;
			return false;
		}

		if (!flatten_locale_json(loader.root(), "", merged_table))
		{
			std::cout << "Load language table failed: unsupported locale JSON shape: "
				<< full_file_path << std::endl;
			return false;
		}
	}

	out_table = std::move(merged_table);
	return true;
}

std::filesystem::path LocalizationManager::resolve_locale_directory(
	const std::string& language
) const
{
	const std::filesystem::path direct_path = _i18n_root / language;
	if (std::filesystem::exists(direct_path))
		return direct_path;

	const std::filesystem::path alias_path =
		_i18n_root / replace_underscores_with_hyphens(language);
	if (std::filesystem::exists(alias_path))
		return alias_path;

	return {};
}

std::string_view LocalizationManager::lookup_translation(
	const TranslationTable& table,
	std::string_view key
) const
{
	const auto iterator = table.find(std::string(key));
	if (iterator == table.end())
		return key;

	return iterator->second;
}

std::string LocalizationManager::map_font_key(
	const std::string& language,
	int point_size
) const
{
	const std::string suffix = "." + std::to_string(point_size);
	if (language == "en")
		return "ui.latin" + suffix;
	if (language == "zh_cn")
		return "ui.zh_hans" + suffix;
	if (language == "ja")
		return "ui.ja" + suffix;

	return {};
}

CachedTexturePtr LocalizationManager::create_text_texture(
	std::string_view key,
	const LocalizedTextStyle& style
)
{
	if (!_renderer)
		return {};

	if (style.point_size <= 0)
	{
		std::cout << "Create text texture failed: invalid point size for key "
			<< key << std::endl;
		return {};
	}

	const std::string font_key = map_font_key(_current_language, style.point_size);
	if (font_key.empty())
	{
		std::cout << "Create text texture failed: font mapping is missing for language "
			<< _current_language << ", size " << style.point_size << std::endl;
		return {};
	}

	TTF_Font* font = ResourceManager::instance()->find_font(font_key);
	if (!font)
	{
		std::cout << "Create text texture failed: font is not loaded: "
			<< font_key << std::endl;
		return {};
	}

	const std::string translated_text(tr(key));
	if (translated_text.empty())
	{
		std::cout << "Create text texture failed: translated text is empty: "
			<< key << std::endl;
		return {};
	}

	SDL_Surface* surface = nullptr;
	if (style.wrap_width > 0)
	{
		surface = TTF_RenderUTF8_Blended_Wrapped(
			font,
			translated_text.c_str(),
			style.color,
			style.wrap_width);
	}
	else
	{
		surface = TTF_RenderUTF8_Blended(
			font,
			translated_text.c_str(),
			style.color);
	}

	if (!surface)
	{
		std::cout << "Create text texture failed: TTF render failed for key "
			<< key << ", error: " << TTF_GetError() << std::endl;
		return {};
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture)
	{
		std::cout << "Create text texture failed: SDL_CreateTextureFromSurface failed for key "
			<< key << ", error: " << SDL_GetError() << std::endl;
		return {};
	}

	return CachedTexturePtr(texture);
}
