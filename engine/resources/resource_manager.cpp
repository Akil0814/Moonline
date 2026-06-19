#include "resource_manager.h"

ResourceManager::ResourceManager()
	: _atlas_manager(_texture_manager)
{
}

bool ResourceManager::init()
{
	return true;
}

bool ResourceManager::begin_atlas_build(const AtlasBuildRequest& request)
{
	return _atlas_manager.begin_build(request);
}

bool ResourceManager::begin_atlas_builds(const std::vector<AtlasBuildRequest>& requests)
{
	return _atlas_manager.begin_builds(requests);
}

bool ResourceManager::commit_prepared_atlas_frame(
	SDL_Renderer* renderer,
	const AtlasFramePreparedResult& result
)
{
	return _atlas_manager.commit_prepared_frame(renderer, result);
}

bool ResourceManager::load_font(
	const std::string& key,
	const std::filesystem::path& file_path,
	int point_size
)
{
	return _font_manager.load_font(key, file_path, point_size);
}

bool ResourceManager::load_sounds(const std::vector<SoundLoadRequest>& requests)
{
	return _audio_manager.load_sounds(requests);
}

bool ResourceManager::load_music(const std::vector<MusicLoadRequest>& requests)
{
	return _audio_manager.load_music(requests);
}

Atlas* ResourceManager::find_atlas(const std::string_view& key) const
{
	return _atlas_manager.find_atlas(key);
}

TTF_Font* ResourceManager::find_font(const std::string_view& key) const
{
	return _font_manager.find_font(key);
}

Mix_Chunk* ResourceManager::find_sound(const std::string_view& key) const
{
	return _audio_manager.find_sound(key);
}

Mix_Music* ResourceManager::find_music(const std::string_view& key) const
{
	return _audio_manager.find_music(key);
}

SDL_Texture* ResourceManager::find_texture(const std::string_view& key) const
{
	return _texture_manager.find_texture(key);
}

AtlasManager& ResourceManager::atlas_manager()
{
	return _atlas_manager;
}

const AtlasManager& ResourceManager::atlas_manager() const
{
	return _atlas_manager;
}

TextureManager& ResourceManager::texture_manager()
{
	return _texture_manager;
}

const TextureManager& ResourceManager::texture_manager() const
{
	return _texture_manager;
}

FontManager& ResourceManager::font_manager()
{
	return _font_manager;
}

const FontManager& ResourceManager::font_manager() const
{
	return _font_manager;
}

AudioManager& ResourceManager::audio_manager()
{
	return _audio_manager;
}

const AudioManager& ResourceManager::audio_manager() const
{
	return _audio_manager;
}

void ResourceManager::clear()
{
	_atlas_manager.clear();
	_texture_manager.clear();
	_font_manager.clear();
	_audio_manager.clear();
}

size_t ResourceManager::resource_count() const
{
	return _atlas_manager.resource_count()
		+ _texture_manager.resource_count()
		+ _font_manager.resource_count()
		+ _audio_manager.resource_count();
}
