#pragma once

#include "audio_settings.h"
#include "sound_playback_scheduler.h"
#include "sound_playback_types.h"
#include "../tools/singleton.h"

#include <string_view>

namespace elysia::audio
{
class AudioService : public elysia::tools::Singleton<AudioService>
{
    friend elysia::tools::Singleton<AudioService>;

public:
    bool init(const AudioSettings& settings);
    void shutdown();

    bool play_sound(const std::string_view& key, int loops = 0);
    SoundRequestResult request_sound(const std::string_view& key, const SoundPlayOptions& options = {});
    void update(double delta_seconds);
    bool cancel_scheduled_sound(ScheduledSoundId id);
    void cancel_all_scheduled_sounds();

    bool set_sound_group_config(SoundGroup group,const SoundGroupConfig& config);
    const SoundGroupConfig& sound_group_config(SoundGroup group) const;

    bool play_music(const std::string_view& key, int loops = -1);
    void stop_music();
    void stop_all_sounds();

    void set_master_volume(int volume);
    void set_music_volume(int volume);
    void set_sound_volume(int volume);

    const AudioSettings& settings() const;

private:
    int start_sound(const std::string_view& key, int loops);
    void apply_volumes() const;
    static int clamp_volume(int volume);
    static int to_mix_volume(int volume);

private:
    AudioSettings _settings{};
    SoundPlaybackScheduler _sound_scheduler;
    bool _initialized = false;
};

}
