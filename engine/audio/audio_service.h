#pragma once

#include "audio_settings.h"
#include "../tools/singleton.h"

#include <string_view>

class AudioService : public Singleton<AudioService>
{
    friend Singleton<AudioService>;

public:
    bool init(const AudioSettings& settings);
    void shutdown();

    bool play_sound(const std::string_view& key, int loops = 0);
    bool play_music(const std::string_view& key, int loops = -1);
    void stop_music();
    void stop_all_sounds();

    void set_master_volume(int volume);
    void set_music_volume(int volume);
    void set_sound_volume(int volume);

    const AudioSettings& settings() const;

private:
    void apply_volumes() const;
    static int clamp_volume(int volume);
    static int to_mix_volume(int volume);

private:
    AudioSettings _settings{};
    bool _initialized = false;
};
