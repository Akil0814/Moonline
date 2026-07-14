#include "../tools/logger.h"
#include "audio_service.h"

#include "../resources/resource_manager.h"

#include <SDL_mixer.h>

#include <algorithm>
namespace elysia::audio
{
bool AudioService::init(const AudioSettings& settings)
{
    _settings.master_volume = clamp_volume(settings.master_volume);
    _settings.music_volume = clamp_volume(settings.music_volume);
    _settings.sound_volume = clamp_volume(settings.sound_volume);

    if (Mix_AllocateChannels(static_cast<int>(kSoundChannelCount)) < static_cast<int>(kSoundChannelCount))
    {
        ELYSIA_LOG_WARN("audio","Audio service failed to allocate sound channels: " << Mix_GetError());
        return false;
    }

    _sound_scheduler.reset();
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
    _sound_scheduler.reset();
    _initialized = false;
}

bool AudioService::play_sound(const std::string_view& key, int loops)
{
    SoundPlayOptions options{};
    options.loops = loops;
    return request_sound(key,options).status == SoundRequestStatus::Started;
}

SoundRequestResult AudioService::request_sound(const std::string_view& key, const SoundPlayOptions& options)
{
    if (!_initialized)
    {
        ELYSIA_LOG_WARN("audio","Play sound failed: audio service is not initialized.");
        return {};
    }

    Mix_Chunk* sound = elysia::resources::ResourceManager::instance()->find_sound(key);
    if (!sound)
    {
        ELYSIA_LOG_WARN("audio","Play sound failed: sound does not exist: " << key);
        return {};
    }

    return _sound_scheduler.request_sound(key,options,
        [this](std::string_view scheduled_key,int scheduled_loops)
        {
            return start_sound(scheduled_key,scheduled_loops);
        },
        [](int channel)
        {
            return Mix_Playing(channel) != 0;
        });
}

void AudioService::update(double delta_seconds)
{
    if (!_initialized)
        return;

    _sound_scheduler.update(delta_seconds,
        [this](std::string_view scheduled_key,int scheduled_loops)
        {
            return start_sound(scheduled_key,scheduled_loops);
        },
        [](int channel)
        {
            return Mix_Playing(channel) != 0;
        });
}

bool AudioService::cancel_scheduled_sound(ScheduledSoundId id)
{
    return _sound_scheduler.cancel_scheduled_sound(id);
}

void AudioService::cancel_all_scheduled_sounds()
{
    _sound_scheduler.cancel_all_scheduled_sounds();
}

bool AudioService::set_sound_group_config(SoundGroup group,const SoundGroupConfig& config)
{
    if (!_sound_scheduler.set_group_config(group,config))
    {
        ELYSIA_LOG_WARN("audio","Sound group config exceeds its fixed maximum.");
        return false;
    }

    return true;
}

const SoundGroupConfig& AudioService::sound_group_config(SoundGroup group) const
{
    return _sound_scheduler.group_config(group);
}

bool AudioService::play_music(const std::string_view& key, int loops)
{
    if (!_initialized)
    {
        ELYSIA_LOG_WARN("audio","Play music failed: audio service is not initialized.");
        return false;
    }

    Mix_Music* music = elysia::resources::ResourceManager::instance()->find_music(key);
    if (!music)
    {
        ELYSIA_LOG_WARN("audio","Play music failed: music does not exist: " << key);
        return false;
    }

    stop_music();

    if (Mix_PlayMusic(music, loops) != 0)
    {
        ELYSIA_LOG_WARN("audio","Play music failed: " << key
            << " error: " << Mix_GetError());
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
    {
        Mix_HaltChannel(-1);
        _sound_scheduler.clear_active_sounds();
    }
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

int AudioService::start_sound(const std::string_view& key, int loops)
{
    Mix_Chunk* sound = elysia::resources::ResourceManager::instance()->find_sound(key);
    if (!sound)
    {
        ELYSIA_LOG_WARN("audio","Play sound failed: sound does not exist: " << key);
        return -1;
    }

    const int channel = Mix_PlayChannel(-1,sound,loops);
    if (channel < 0)
    {
        ELYSIA_LOG_WARN("audio","Play sound failed: " << key
            << " error: " << Mix_GetError());
    }
    return channel;
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
