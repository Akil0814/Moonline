#pragma once

#include "localized_text_style.h"

#include <SDL.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

struct SdlTextureDeleter
{
	void operator()(SDL_Texture* texture) const;
};

using CachedTexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

struct TextTextureCacheKey
{
	std::string language;
	std::string translation_key;
	int point_size = 0;
	SDL_Color color{ 255, 255, 255, 255 };
	int wrap_width = 0;

	bool operator==(const TextTextureCacheKey& other) const;
};

struct TextTextureCacheKeyHash
{
	size_t operator()(const TextTextureCacheKey& key) const;
};

class TextTextureCache
{
public:
	using TextureFactory = std::function<CachedTexturePtr()>;

	SDL_Texture* get_or_create(
		const std::string& language,
		std::string_view translation_key,
		const LocalizedTextStyle& style,
		const TextureFactory& texture_factory
	);

	void clear();

private:
	std::unordered_map<TextTextureCacheKey, CachedTexturePtr, TextTextureCacheKeyHash> _textures;
};
