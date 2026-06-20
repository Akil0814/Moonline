#include "audio_service.h"

#include "../resources/resource_manager.h"

#include <SDL_mixer.h>

#include <algorithm>
#include <iostream>

namespace elysia::audio
{
bool AudioService::init(const AudioSettings& settings)
{
    _settings.master_volume = clamp_volume(settings.master_volume);
    _settings.music_volume = clamp_volume(settings.music_volume);
    _settings.sound_volume = clamp_volume(settings.sound_volume);

    _initialized = true;
    apply_volumes();
    return true;
}

void AudioService::shutdown()
{
    if (!_initialized)
        return;

    stop_music();
    stop_all_sounds();
    _initialized = false;
}

bool AudioService::play_sound(const std::string_view& key, int loops)
{
    if (!_initialized)
    {
        std::cout << "Play sound failed: audio service is not initialized." << std::endl;
        return false;
    }

    Mix_Chunk* sound = elysia::resources::ResourceManager::instance()->find_sound(key);
    if (!sound)
    {
        std::cout << "Play sound failed: sound does not exist: " << key << std::endl;
        return false;
    }

    if (Mix_PlayChannel(-1, sound, loops) < 0)
    {
        std::cout << "Play sound failed: " << key
            << " error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

bool AudioService::play_music(const std::string_view& key, int loops)
{
    if (!_initialized)
    {
        std::cout << "Play music failed: audio service is not initialized." << std::endl;
        return false;
    }

    Mix_Music* music = elysia::resources::ResourceManager::instance()->find_music(key);
    if (!music)
    {
        std::cout << "Play music failed: music does not exist: " << key << std::endl;
        return false;
    }

    stop_music();

    if (Mix_PlayMusic(music, loops) != 0)
    {
        std::cout << "Play music failed: " << key
            << " error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

void AudioService::stop_music()
{
    if (_initialized)
        Mix_HaltMusic();
}

void AudioService::stop_all_sounds()
{
    if (_initialized)
        Mix_HaltChannel(-1);
}

void AudioService::set_master_volume(int volume)
{
    _settings.master_volume = clamp_volume(volume);
    apply_volumes();
}

void AudioService::set_music_volume(int volume)
{
    _settings.music_volume = clamp_volume(volume);
    apply_volumes();
}

void AudioService::set_sound_volume(int volume)
{
    _settings.sound_volume = clamp_volume(volume);
    apply_volumes();
}

const AudioSettings& AudioService::settings() const
{
    return _settings;
}

void AudioService::apply_volumes() const
{
    if (!_initialized)
        return;

    const int effective_music = (_settings.master_volume * _settings.music_volume) / 100;
    const int effective_sound = (_settings.master_volume * _settings.sound_volume) / 100;

    Mix_VolumeMusic(to_mix_volume(effective_music));
    Mix_Volume(-1, to_mix_volume(effective_sound));
}

int AudioService::clamp_volume(int volume)
{
    return std::clamp(volume, 0, 100);
}

int AudioService::to_mix_volume(int volume)
{
    return (clamp_volume(volume) * MIX_MAX_VOLUME) / 100;
}

}
