#include "../engine/audio/sound_playback_scheduler.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unordered_set>

namespace
{
using namespace std::chrono_literals;
using elysia::audio::SoundGroup;
using elysia::audio::SoundGroupConfig;
using elysia::audio::SoundPlayOptions;
using elysia::audio::SoundPlaybackScheduler;
using elysia::audio::SoundRequestStatus;

void require(bool condition,const char* message)
{
    if (condition)
        return;
    std::cerr << "FAILED: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

struct FakeChannels
{
    int next_channel = 0;
    int start_count = 0;
    std::unordered_set<int> playing;

    int start(std::string_view,int)
    {
        const int channel = next_channel++;
        playing.insert(channel);
        ++start_count;
        return channel;
    }

    [[nodiscard]] bool is_playing(int channel) const
    {
        return playing.contains(channel);
    }
};

SoundPlayOptions options_for(SoundGroup group)
{
    SoundPlayOptions options{};
    options.group = group;
    return options;
}

void test_group_limits_and_finished_channel_cleanup()
{
    SoundPlaybackScheduler scheduler;
    FakeChannels channels;
    const auto start = [&channels](std::string_view key,int loops) { return channels.start(key,loops); };
    const auto playing = [&channels](int channel) { return channels.is_playing(channel); };
    const SoundPlayOptions ui = options_for(SoundGroup::Ui);

    for (int index = 0; index < 4; ++index)
        require(scheduler.request_sound("ui.click",ui,start,playing).status == SoundRequestStatus::Started,
            "UI group must accept sounds until its hard limit");
    require(scheduler.request_sound("ui.click",ui,start,playing).status == SoundRequestStatus::Rejected,
        "UI group must reject sounds over its hard limit");

    channels.playing.erase(0);
    require(scheduler.request_sound("ui.click",ui,start,playing).status == SoundRequestStatus::Started,
        "a finished channel must be reclaimed before checking group limits");
}

void test_configurable_limits_and_global_budget()
{
    SoundPlaybackScheduler scheduler;
    FakeChannels channels;
    const auto start = [&channels](std::string_view key,int loops) { return channels.start(key,loops); };
    const auto playing = [&channels](int channel) { return channels.is_playing(channel); };

    SoundGroupConfig ui_config{};
    ui_config.max_simultaneous = 2;
    require(scheduler.set_group_config(SoundGroup::Ui,ui_config),"a group limit below its hard limit must be accepted");
    ui_config.max_simultaneous = 5;
    require(!scheduler.set_group_config(SoundGroup::Ui,ui_config),"a group limit above its hard limit must be rejected");

    const auto ui = options_for(SoundGroup::Ui);
    require(scheduler.request_sound("ui.one",ui,start,playing).status == SoundRequestStatus::Started,"first configured UI slot must start");
    require(scheduler.request_sound("ui.two",ui,start,playing).status == SoundRequestStatus::Started,"second configured UI slot must start");
    require(scheduler.request_sound("ui.three",ui,start,playing).status == SoundRequestStatus::Rejected,"configured UI limit must be enforced");

    SoundPlaybackScheduler global_scheduler;
    FakeChannels global_channels;
    const auto global_start = [&global_channels](std::string_view key,int loops) { return global_channels.start(key,loops); };
    const auto global_playing = [&global_channels](int channel) { return global_channels.is_playing(channel); };
    for (const auto group : { SoundGroup::Ui,SoundGroup::Gameplay,SoundGroup::Ambient,SoundGroup::Extra })
    {
        const SoundPlayOptions options = options_for(group);
        const std::size_t count = elysia::audio::sound_group_hard_limit(group);
        for (std::size_t index = 0; index < count; ++index)
            require(global_scheduler.request_sound("sound." + std::to_string(global_channels.start_count),options,global_start,global_playing).status == SoundRequestStatus::Started,
                "all fixed group budgets must fit inside the global channel budget");
    }
    require(global_channels.start_count == static_cast<int>(elysia::audio::kSoundChannelCount),
        "the four fixed group budgets must consume exactly 24 channels");
}

void test_per_key_cooldown()
{
    SoundPlaybackScheduler scheduler;
    FakeChannels channels;
    const auto start = [&channels](std::string_view key,int loops) { return channels.start(key,loops); };
    const auto playing = [&channels](int channel) { return channels.is_playing(channel); };

    SoundGroupConfig config{};
    config.cooldown = 100ms;
    require(scheduler.set_group_config(SoundGroup::Ui,config),"valid cooldown config must be accepted");
    const auto ui = options_for(SoundGroup::Ui);
    require(scheduler.request_sound("ui.click",ui,start,playing).status == SoundRequestStatus::Started,"first key request must start");
    require(scheduler.request_sound("ui.click",ui,start,playing).status == SoundRequestStatus::Rejected,"same key must be cooled down");
    require(scheduler.request_sound("ui.focus",ui,start,playing).status == SoundRequestStatus::Started,"different key must not share cooldown");
    scheduler.update(0.1,start,playing);
    require(scheduler.request_sound("ui.click",ui,start,playing).status == SoundRequestStatus::Started,"key cooldown must expire after its interval");
}

void test_delayed_requests_cancellation_and_capacity_drop()
{
    SoundPlaybackScheduler scheduler;
    FakeChannels channels;
    const auto start = [&channels](std::string_view key,int loops) { return channels.start(key,loops); };
    const auto playing = [&channels](int channel) { return channels.is_playing(channel); };

    SoundPlayOptions delayed = options_for(SoundGroup::Gameplay);
    delayed.start_delay = 100ms;
    const auto request = scheduler.request_sound("gameplay.explosion",delayed,start,playing);
    require(request.status == SoundRequestStatus::Scheduled && request.scheduled_id,"delayed request must return a cancellable id");
    scheduler.update(0.099,start,playing);
    require(channels.start_count == 0,"delayed sound must not start before its due time");
    scheduler.update(0.001,start,playing);
    require(channels.start_count == 1,"delayed sound must start at its due time");

    const auto cancelled = scheduler.request_sound("gameplay.cancelled",delayed,start,playing);
    require(cancelled.scheduled_id && scheduler.cancel_scheduled_sound(*cancelled.scheduled_id),"scheduled request must be cancellable");
    scheduler.update(0.1,start,playing);
    require(channels.start_count == 1,"cancelled request must never start");

    SoundPlaybackScheduler full_scheduler;
    FakeChannels full_channels;
    const auto full_start = [&full_channels](std::string_view key,int loops) { return full_channels.start(key,loops); };
    const auto full_playing = [&full_channels](int channel) { return full_channels.is_playing(channel); };
    const auto ui = options_for(SoundGroup::Ui);
    for (int index = 0; index < 4; ++index)
        require(full_scheduler.request_sound("ui.active",ui,full_start,full_playing).status == SoundRequestStatus::Started,"fill UI group before delayed capacity test");
    SoundPlayOptions delayed_ui = ui;
    delayed_ui.start_delay = 1ms;
    require(full_scheduler.request_sound("ui.delayed",delayed_ui,full_start,full_playing).status == SoundRequestStatus::Scheduled,"full group request can wait for its scheduled time");
    full_scheduler.update(0.001,full_start,full_playing);
    require(full_channels.start_count == 4,"due sound must be dropped when its group is full");
}
}

int main()
{
    test_group_limits_and_finished_channel_cleanup();
    test_configurable_limits_and_global_budget();
    test_per_key_cooldown();
    test_delayed_requests_cancellation_and_capacity_drop();
    std::cout << "sound playback scheduler tests passed\n";
    return EXIT_SUCCESS;
}
