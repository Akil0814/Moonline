#include "text_texture_cache.h"

#include <functional>

namespace elysia::localization
{
namespace
{
inline void hash_combine(size_t& seed, size_t value)
{
	seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
}

void SdlTextureDeleter::operator()(SDL_Texture* texture) const
{
	if (texture)
		SDL_DestroyTexture(texture);
}

bool TextTextureCacheKey::operator==(const TextTextureCacheKey& other) const
{
	return language == other.language
		&& translation_key == other.translation_key
		&& is_raw_text == other.is_raw_text
		&& point_size == other.point_size
		&& wrap_width == other.wrap_width
		&& color == other.color;
}

size_t TextTextureCacheKeyHash::operator()(const TextTextureCacheKey& key) const
{
	size_t seed = std::hash<std::string>{}(key.language);
	hash_combine(seed, std::hash<std::string>{}(key.translation_key));
	hash_combine(seed, std::hash<bool>{}(key.is_raw_text));
	hash_combine(seed, std::hash<int>{}(key.point_size));
	hash_combine(seed, std::hash<int>{}(key.wrap_width));
	hash_combine(seed, std::hash<unsigned int>{}(key.color.r));
	hash_combine(seed, std::hash<unsigned int>{}(key.color.g));
	hash_combine(seed, std::hash<unsigned int>{}(key.color.b));
	hash_combine(seed, std::hash<unsigned int>{}(key.color.a));
	return seed;
}

SDL_Texture* TextTextureCache::get_or_create(
	const std::string& language,
	std::string_view translation_key,
	const LocalizedTextStyle& style,
	const TextureFactory& texture_factory
)
{
	TextTextureCacheKey key;
	key.language = language;
	key.translation_key = std::string(translation_key);
	key.is_raw_text = false;
	key.point_size = style.point_size;
	key.color = style.color;
	key.wrap_width = style.wrap_width;

	const auto found = _textures.find(key);
	if (found != _textures.end())
		return found->second.get();

	if (!texture_factory)
		return nullptr;

	CachedTexturePtr created_texture = texture_factory();
	if (!created_texture)
		return nullptr;

	SDL_Texture* raw_texture = created_texture.get();
	_textures.emplace(std::move(key), std::move(created_texture));
	return raw_texture;
}

SDL_Texture* TextTextureCache::get_or_create_raw(
	const std::string& language,
	std::string_view raw_text,
	const LocalizedTextStyle& style,
	const TextureFactory& texture_factory
)
{
	TextTextureCacheKey key;
	key.language = language;
	key.translation_key = std::string(raw_text);
	key.is_raw_text = true;
	key.point_size = style.point_size;
	key.color = style.color;
	key.wrap_width = style.wrap_width;

	const auto found = _textures.find(key);
	if (found != _textures.end())
		return found->second.get();

	if (!texture_factory)
		return nullptr;

	CachedTexturePtr created_texture = texture_factory();
	if (!created_texture)
		return nullptr;

	SDL_Texture* raw_texture = created_texture.get();
	_textures.emplace(std::move(key), std::move(created_texture));
	return raw_texture;
}

void TextTextureCache::clear()
{
	_textures.clear();
}

}
