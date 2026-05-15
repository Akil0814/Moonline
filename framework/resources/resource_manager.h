#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

#include "atlas.h"
#include "resource_types.h"
#include "texture_loader.h"
#include "../base/singleton.h"


typedef std::unordered_map<std::string, std::unique_ptr<Atlas>> AtlasPool;
typedef std::unordered_map<std::string, TTF_Font*> FontPool;
typedef std::unordered_map<std::string, Mix_Chunk*> SoundPool;
typedef std::unordered_map<std::string, Mix_Music*> MusicPool;
typedef std::unordered_map<std::string, TexturePtr> TexturePool;

class ResourceManager : public Singleton<ResourceManager>
{
	friend Singleton<ResourceManager>;
public:
	bool load_atlas(SDL_Renderer* renderer, const AtlasLoadRequest& request);
	bool load_atlases(
		SDL_Renderer* renderer,
		const std::vector<AtlasLoadRequest>& requests
	);

	Atlas* find_atlas(const std::string_view& key) const;
	TTF_Font* find_font(const std::string_view& key) const;
	Mix_Chunk* find_sound(const std::string_view& key) const;
	Mix_Music* find_music(const std::string_view& key) const;
	SDL_Texture* find_texture(const std::string_view& key)const;

private:
	bool collect_frame_paths(
		const AtlasLoadRequest& request,
		std::vector<std::filesystem::path>& frame_paths
	) const;
	std::string make_texture_key(
		const std::string& atlas_key,
		size_t frame_index
	) const;

private:
	AtlasPool _atlas_pool;
	FontPool _font_pool;
	SoundPool _sound_pool;
	MusicPool _music_pool;
	TexturePool _texture_pool;

};
