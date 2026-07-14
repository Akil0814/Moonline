#define SDL_MAIN_HANDLED

#include "../engine/animation/animation_manager.h"
#include "../engine/core/time.h"
#include "../engine/effects/effect_manager.h"
#include "../engine/resources/atlas/atlas.h"
#include "../engine/scene/scene.h"
#include "../engine/scene/scene_manager.h"

#include <SDL.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
void require(bool condition, const char* message)
{
    if (condition)
        return;

    std::cerr << "FAILED: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

class TestScene final : public elysia::scene::Scene
{
public:
    void on_enter(const elysia::scene::ScenePayload&) override {}
    void on_exit() override {}
    void reset() override {}
};

class SecondTestScene final : public elysia::scene::Scene
{
public:
    void on_enter(const elysia::scene::ScenePayload&) override {}
    void on_exit() override {}
    void reset() override {}
};

void register_animation_effect(SDL_Texture* texture, bool loop, const char* suffix)
{
    using namespace elysia;
    static resources::Atlas atlas("animation_effect_test.atlas");
    atlas.clear();
    require(atlas.add_textures(texture, texture, texture), "test atlas must accept textures");

    resources::AnimationBuildRequest animation_request;
    animation_request.animation_key = std::string("animation_effect_test.animation.") + suffix;
    animation_request.atlas_key = "animation_effect_test.atlas";
    animation_request.fps = 10.0;
    animation_request.loop = loop;
    require(animation::AnimationManager::instance()->register_animation(animation_request, &atlas),
        "animation effect test animation must register");

    resources::AnimationEffectBuildRequest effect_request;
    effect_request.effect_key = std::string("animation_effect_test.effect.") + suffix;
    effect_request.animation_key = animation_request.animation_key;
    effect_request.default_size = core::Vector2(20.0f, 10.0f);
    effect_request.default_angle_degrees = 15.0;
    require(effects::EffectManager::instance()->register_animation_effect(effect_request),
        "animation effect test definition must register");
}
}

int main()
{
    using namespace elysia;

    require(SDL_Init(SDL_INIT_VIDEO) == 0, "animation effect tests must initialize SDL video");
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 20, 10, 32, SDL_PIXELFORMAT_RGBA32);
    require(surface != nullptr, "animation effect tests must create a surface");
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    require(renderer != nullptr, "animation effect tests must create a software renderer");
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 20, 10);
    require(texture != nullptr, "animation effect tests must create a texture");

    register_animation_effect(texture, false, "oneshot");
    effects::AnimationEffectSpawnRequest request;
    request.effect_key = "animation_effect_test.effect.oneshot";
    request.position = core::Vector2(100.0f, 50.0f);
    request.anchor = effects::EffectAnchor::Center;
    request.start_delay_seconds = 0.2;

    std::unique_ptr<effects::AnimationEffect> effect =
        effects::EffectManager::instance()->create_animation_effect(request);
    require(effect != nullptr, "animation effect must be created");
    require(effect->size() == core::Vector2(20.0f, 10.0f), "definition default size must apply");
    require(effect->position() == core::Vector2(90.0f, 45.0f), "center anchor must resolve from final size");

    resources::AnimationBuildRequest natural_animation_request;
    natural_animation_request.animation_key = "animation_effect_test.animation.natural";
    natural_animation_request.atlas_key = "animation_effect_test.atlas";
    natural_animation_request.fps = 10.0;
    natural_animation_request.loop = false;
    require(animation::AnimationManager::instance()->register_animation(natural_animation_request,
        animation::AnimationManager::instance()->find_definition("animation_effect_test.animation.oneshot")->atlas),
        "natural-size animation must register");
    resources::AnimationEffectBuildRequest natural_effect_request;
    natural_effect_request.effect_key = "animation_effect_test.effect.natural";
    natural_effect_request.animation_key = natural_animation_request.animation_key;
    require(effects::EffectManager::instance()->register_animation_effect(natural_effect_request),
        "natural-size effect definition must register");
    effects::AnimationEffectSpawnRequest natural_request;
    natural_request.effect_key = natural_effect_request.effect_key;
    std::unique_ptr<effects::AnimationEffect> natural_effect =
        effects::EffectManager::instance()->create_animation_effect(natural_request);
    require(natural_effect && natural_effect->size() == core::Vector2(20.0f, 10.0f),
        "first-frame dimensions must be the default-size fallback");

    int started = 0;
    int finished = 0;
    int callbacks = 0;
    effect->set_on_started([&started](effects::AnimationEffect&) { ++started; });
    effect->set_on_finished([&finished](effects::AnimationEffect&) { ++finished; });
    effect->schedule_callback(0.1, [&callbacks](effects::AnimationEffect&) { callbacks = callbacks * 10 + 1; });
    effect->schedule_callback(0.1, [&callbacks](effects::AnimationEffect&) { callbacks = callbacks * 10 + 2; });

    std::vector<core::RenderCommand> commands;
    effect->submit_render_commands(commands);
    require(commands.empty(), "delayed animation effect must not render before playback starts");
    effect->update(0.1);
    require(started == 0 && callbacks == 0, "delay must block callbacks and start event");
    effect->update(0.1);
    require(started == 1 && effect->is_started(), "effect must start after the delay");
    effect->update(0.1);
    require(callbacks == 12, "callbacks at the same time must preserve registration order");
    effect->update(0.2);
    require(finished == 1 && effect->is_destroyed(), "non-looping effect must finish once and destroy itself");

    register_animation_effect(texture, true, "loop");
    effects::AnimationEffectSpawnRequest loop_request;
    loop_request.effect_key = "animation_effect_test.effect.loop";
    std::unique_ptr<effects::AnimationEffect> loop_effect =
        effects::EffectManager::instance()->create_animation_effect(loop_request);
    require(loop_effect != nullptr, "looping animation effect must be created");
    int loop_finished = 0;
    loop_effect->set_on_finished([&loop_finished](effects::AnimationEffect&) { ++loop_finished; });
    loop_effect->set_time_scale(0.5);
    loop_effect->schedule_callback(0.1, [&callbacks](effects::AnimationEffect&) { ++callbacks; });
    loop_effect->update(0.1);
    require(callbacks == 12, "local time scale must slow scheduled callbacks");
    loop_effect->update(0.1);
    require(callbacks == 13, "local time scale must apply to callback time");
    core::Time::instance()->set_time_scale(0.5);
    core::Time::instance()->begin_frame(0.2);
    loop_effect->update(core::Time::instance()->delta());
    require(loop_finished == 0 && !loop_effect->is_destroyed(), "looping effect must not finish");

    int cancelled_callbacks = 0;
    loop_effect->schedule_callback(0.0, [&cancelled_callbacks](effects::AnimationEffect&) { ++cancelled_callbacks; });
    loop_effect->destroy();
    loop_effect->update(1.0);
    require(cancelled_callbacks == 0, "destroyed effects must cancel pending callbacks");

    effects::AnimationEffectSpawnRequest scene_request;
    scene_request.effect_key = "animation_effect_test.effect.oneshot";
    scene_request.start_delay_seconds = 0.1;
    int spawned_started = 0;
    int spawned_finished = 0;
    int spawned_callbacks = 0;
    scene_request.on_started = [&spawned_started](effects::AnimationEffect&) { ++spawned_started; };
    scene_request.on_finished = [&spawned_finished](effects::AnimationEffect&) { ++spawned_finished; };
    scene_request.scheduled_callbacks.push_back({
        .delay_seconds = 0.1,
        .callback = [&spawned_callbacks](effects::AnimationEffect&) { ++spawned_callbacks; }
    });

    effects::EffectManager* effect_manager = effects::EffectManager::instance();
    require(!effect_manager->spawn_animation_effect(scene_request),
        "animation effect spawning must fail without an active scene");

    scene::SceneManager scene_manager;
    constexpr scene::SceneKey first_scene_key = 1001;
    constexpr scene::SceneKey second_scene_key = 1002;
    scene_manager.register_scene<TestScene>(first_scene_key);
    scene_manager.register_scene<SecondTestScene>(second_scene_key);
    scene_manager.start(first_scene_key);
    effects::AnimationEffectSpawnRequest unknown_request;
    unknown_request.effect_key = "animation_effect_test.effect.unknown";
    require(!effect_manager->spawn_animation_effect(unknown_request),
        "animation effect spawning must fail for an unknown effect key");
    require(effect_manager->spawn_animation_effect(scene_request),
        "effect manager must attach effects to the active scene");
    scene_manager.on_update(0.1);
    scene_manager.on_update(0.1);
    scene_manager.on_update(0.2);
    require(spawned_started == 1 && spawned_callbacks == 1 && spawned_finished == 1,
        "spawn requests must configure lifecycle and scheduled callbacks");

    scene::SceneRequest switch_request;
    switch_request.type = scene::SceneRequestType::Switch;
    switch_request.target = second_scene_key;
    scene_manager.on_scene_request(switch_request);
    scene_manager.on_update(0.0);
    int second_scene_started = 0;
    scene_request.start_delay_seconds = 0.0;
    scene_request.on_started = [&second_scene_started](effects::AnimationEffect&) { ++second_scene_started; };
    scene_request.on_finished = {};
    scene_request.scheduled_callbacks.clear();
    require(effect_manager->spawn_animation_effect(scene_request),
        "effect manager must rebind to the scene selected by a switch");
    scene_manager.on_update(0.0);
    require(second_scene_started == 1, "effects spawned after a switch must update in the new scene");
    scene_manager.shutdown();
    require(!effect_manager->spawn_animation_effect(scene_request),
        "animation effect spawning must fail after active scene shutdown");

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_Quit();
    return EXIT_SUCCESS;
}
