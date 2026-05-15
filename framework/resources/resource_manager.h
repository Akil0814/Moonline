#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <unordered_map>
#include <string>
#include <string_view>

#include "atlas.h"
#include "../base/singleton.h"


typedef std::unordered_map<std::string, Atlas*> AtlasPool;
typedef std::unordered_map<std::string, TTF_Font*> FontPool;
typedef std::unordered_map<std::string, Mix_Chunk*> SoundPool;
typedef std::unordered_map<std::string, Mix_Music*> MusicPool;
typedef std::unordered_map<std::string, SDL_Texture*> TexturePool;

class ResourceManager : public Singleton<ResourceManager>
{
	friend Singleton<ResourceManager>;
public:
	Atlas* find_atlas(const std::string_view& key) const;
	TTF_Font* get_font(const std::string_view& key) const;
	Mix_Chunk* get_sound(const std::string_view& key) const;
	Mix_Music* get_music(const std::string_view& key) const;
	SDL_Texture* get_texture(const std::string_view& key)const;

private:
	AtlasPool _atlas_pool;
	FontPool _font_pool;
	SoundPool _sound_pool;
	MusicPool _music_pool;
	TexturePool _texture_pool;

};