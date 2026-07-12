#include "../../tools/logger.h"
#include "texture_manager.h"
#include <utility>

namespace elysia::resources
{
bool TextureManager::store_texture(const std::string& key, TexturePtr texture)
{
	if (key.empty())
	{
		ELYSIA_LOG_ERROR("resource","Store texture failed: key is empty.");
		return false;
	}

	if (!texture)
	{
		ELYSIA_LOG_ERROR("resource","Store texture failed: texture is null: "
			<< key);
		return false;
	}

	if (_texture_pool.contains(key))
		return true;

	_texture_pool.emplace(key, std::move(texture));
	return true;
}

SDL_Texture* TextureManager::find_texture(const std::string_view& key) const
{
	TexturePool::const_iterator iterator = _texture_pool.find(std::string(key));
	if (iterator == _texture_pool.end())
		return nullptr;

	return iterator->second.get();
}

void TextureManager::clear()
{
	_texture_pool.clear();
}

size_t TextureManager::resource_count() const
{
	return _texture_pool.size();
}

}
