#include "sound_playback_scheduler.h"

#include <algorithm>

namespace elysia::audio
{
bool SoundPlaybackScheduler::set_group_config(SoundGroup group,const SoundGroupConfig& config)
{
    if (config.max_simultaneous && *config.max_simultaneous > sound_group_hard_limit(group))
        return false;
    if (config.cooldown.count() < 0)
        return false;

    _group_configs[sound_group_index(group)] = config;
    return true;
}

const SoundGroupConfig& SoundPlaybackScheduler::group_config(SoundGroup group) const
{
    return _group_configs[sound_group_index(group)];
}

SoundRequestResult SoundPlaybackScheduler::request_sound(
    std::string_view key,
    const SoundPlayOptions& options,
    const StartSoundCallback& start_sound,
    const ChannelPlayingCallback& is_channel_playing,
    const StopSoundCallback& stop_sound)
{
    if (key.empty())
        return {};

    if (options.start_delay.count() <= 0)
    {
        return { try_start_sound(key,options,start_sound,is_channel_playing,stop_sound)
            ? SoundRequestStatus::Started
            : SoundRequestStatus::Rejected, std::nullopt };
    }

    const ScheduledSoundId id = _next_scheduled_id++;
    _pending_sounds.push_back({ id,std::string(key),options,_elapsed_seconds + options.start_delay.count() / 1000.0 });
    return { SoundRequestStatus::Scheduled,id };
}

void SoundPlaybackScheduler::update(
    double delta_seconds,
    const StartSoundCallback& start_sound,
    const ChannelPlayingCallback& is_channel_playing,
    const StopSoundCallback& stop_sound)
{
    _elapsed_seconds += std::max(0.0,delta_seconds);
    prune_finished_sounds(is_channel_playing);

    auto pending = _pending_sounds.begin();
    while (pending != _pending_sounds.end())
    {
        if (pending->due_time_seconds > _elapsed_seconds)
        {
            ++pending;
            continue;
        }

        (void)try_start_sound(pending->key,pending->options,start_sound,is_channel_playing,stop_sound);
        pending = _pending_sounds.erase(pending);
    }
}

bool SoundPlaybackScheduler::cancel_scheduled_sound(ScheduledSoundId id)
{
    const auto pending = std::find_if(_pending_sounds.begin(),_pending_sounds.end(),
        [id](const PendingSound& sound) { return sound.id == id; });
    if (pending == _pending_sounds.end())
        return false;

    _pending_sounds.erase(pending);
    return true;
}

void SoundPlaybackScheduler::cancel_all_scheduled_sounds()
{
    _pending_sounds.clear();
}

void SoundPlaybackScheduler::clear_active_sounds()
{
    _active_sounds.clear();
}

void SoundPlaybackScheduler::reset()
{
    _active_sounds.clear();
    _pending_sounds.clear();
    _last_started_seconds_by_key.clear();
    _elapsed_seconds = 0.0;
    _next_scheduled_id = 1;
}

bool SoundPlaybackScheduler::try_start_sound(
    std::string_view key,
    const SoundPlayOptions& options,
    const StartSoundCallback& start_sound,
    const ChannelPlayingCallback& is_channel_playing,
    const StopSoundCallback& stop_sound)
{
    prune_finished_sounds(is_channel_playing);

    const SoundGroupConfig& config = group_config(options.group);
    const auto last_started = _last_started_seconds_by_key.find(std::string(key));
    const double cooldown_seconds = config.cooldown.count() / 1000.0;
    if (last_started != _last_started_seconds_by_key.end()
        && _elapsed_seconds - last_started->second < cooldown_seconds)
        return false;

    const std::size_t group_limit = config.max_simultaneous.value_or(sound_group_hard_limit(options.group));
    if (active_count(options.group) >= group_limit)
    {
        if (config.overflow_policy == SoundOverflowPolicy::IgnoreNew || !stop_sound)
            return false;

        const auto oldest = std::find_if(_active_sounds.begin(),_active_sounds.end(),
            [&options](const ActiveSound& sound) { return sound.group == options.group; });
        if (oldest == _active_sounds.end())
            return false;

        stop_sound(oldest->channel);
        _active_sounds.erase(oldest);
    }

    if (_active_sounds.size() >= kSoundChannelCount)
        return false;

    const int channel = start_sound(key,options.loops.value_or(0));
    if (channel < 0)
        return false;

    _active_sounds.push_back({ channel,options.group });
    _last_started_seconds_by_key[std::string(key)] = _elapsed_seconds;
    return true;
}

void SoundPlaybackScheduler::prune_finished_sounds(const ChannelPlayingCallback& is_channel_playing)
{
    std::erase_if(_active_sounds,[&is_channel_playing](const ActiveSound& sound)
    {
        return !is_channel_playing(sound.channel);
    });
}

std::size_t SoundPlaybackScheduler::active_count(SoundGroup group) const
{
    return static_cast<std::size_t>(std::count_if(_active_sounds.begin(),_active_sounds.end(),
        [group](const ActiveSound& sound) { return sound.group == group; }));
}
}
