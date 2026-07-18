#define SDL_MAIN_HANDLED

#include "engine/assist/engine_assist_cache.h"
#include "engine/assist/engine_assist_catalog.h"
#include "engine/assist/engine_assist_keys.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/content_runtime_cleanup.h"
#include "engine/ui/widgets/image/ui_animation.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <chrono>
#include <array>
#include <filesystem>

namespace
{
using moonline::tests::require;

class AssistCacheFixture
{
public:
    AssistCacheFixture()
    {
        require(SDL_Init(SDL_INIT_VIDEO) == 0,
            "Engine assist cache tests must initialize SDL video");
        require((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == IMG_INIT_PNG,
            "Engine assist cache tests must initialize PNG support");
        require(TTF_Init() == 0,
            "Engine assist cache tests must initialize SDL_ttf");

        _surface = SDL_CreateRGBSurfaceWithFormat(0, 128, 128, 32, SDL_PIXELFORMAT_RGBA32);
        require(_surface != nullptr,
            "Engine assist cache tests must create a software surface");
        _renderer = SDL_CreateSoftwareRenderer(_surface);
        require(_renderer != nullptr,
            "Engine assist cache tests must create a software renderer");
    }

    ~AssistCacheFixture()
    {
        SDL_DestroyRenderer(_renderer);
        SDL_FreeSurface(_surface);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    [[nodiscard]] SDL_Renderer* renderer() const noexcept { return _renderer; }

private:
    SDL_Surface* _surface = nullptr;
    SDL_Renderer* _renderer = nullptr;
};
}

int main()
{
    AssistCacheFixture fixture;
    const std::filesystem::path source_root = MOONLINE_SOURCE_DIR;
    elysia::assist::EngineAssistCatalog catalog(source_root);
    elysia::assist::EngineAssistCache cache;
    constexpr std::array default_point_sizes{10,20,30,40,50,60,70};

    const auto initialized = cache.initialize(
        fixture.renderer(),
        catalog,
        default_point_sizes);
    require(initialized.has_value(), "Engine assist cache must load all repository assist resources");
    require(cache.initialized(), "successful cache initialization must publish a live cache");
    require(cache.texture_count() == 6, "cache must own all six Engine textures");
    require(cache.font_count() == 35, "cache must own five font faces at seven fixed sizes");
    require(cache.locale_count() == 5, "cache must own all five Engine translation tables");
    require(cache.find_texture(
            elysia::assist::asset_keys::ElysiaWhiteTexture) != nullptr,
        "cache must expose the Engine startup logo by stable key");
    require(cache.find_texture("engine.test.sprite") != nullptr,
        "cache must expose the Engine test sprite by stable key");
    require(cache.find_font("zh-Hans", 30) != nullptr,
        "cache must expose Engine fonts by locale and point size");

    elysia::assist::EngineAssistCache custom_size_cache;
    constexpr std::array custom_point_sizes{24};
    require(custom_size_cache.initialize(
            fixture.renderer(),
            catalog,
            custom_point_sizes).has_value()
            && custom_size_cache.font_count() == 5
            && custom_size_cache.find_font("en",24) != nullptr,
        "cache must dynamically load Application-requested point sizes");
    custom_size_cache.shutdown();
    require(cache.find_translation("ja", "engine.settings.title") != nullptr,
        "cache must expose parsed Engine translations");
    require(elysia::assist::EngineAssistCache::map_project_locale("zh_cn") == "zh-Hans",
        "project Simplified Chinese must map to the Engine BCP-47 locale");
    require(cache.animation_count() == 1, "cache must register the Engine test animation");
    const auto* animation_definition = cache.find_animation("engine.test.idle");
    require(animation_definition != nullptr && animation_definition->atlas != nullptr
            && animation_definition->atlas->size() == 8 && animation_definition->fps == 8.0
            && animation_definition->loop,
        "cache must expose the complete Engine test animation definition");
    const auto* first_frame = animation_definition->atlas->frame_at(0);
    const auto* last_frame = animation_definition->atlas->frame_at(7);
    require(first_frame != nullptr && last_frame != nullptr
            && first_frame->_source_rect.has_value() && last_frame->_source_rect.has_value()
            && first_frame->_source_rect->width() == 32.0f && first_frame->_source_rect->height() == 32.0f
            && last_frame->_source_rect->x() == 224.0f,
        "Engine test animation atlas must expose eight 32 px source rectangles");
    const auto animation = cache.create_animation("engine.test.idle");
    require(animation != nullptr && animation->current_frame_index() == 0
            && animation->current_frame() == first_frame,
        "cache must create an initialized Engine test animation instance");

    elysia::ui::UiAnimation ui_animation(
        elysia::core::Rect{ 0.0f,0.0f,32.0f,32.0f });
    require(ui_animation.set_engine_animation(cache,"engine.test.idle")
            && ui_animation.is_looping(),
        "UiAnimation must bind looping Engine Assist animations without AnimationManager");

    elysia::loading::clear_loaded_content();
    require(cache.find_texture(
            elysia::assist::asset_keys::ElysiaWhiteTexture) != nullptr
            && cache.find_font("en", 20) != nullptr,
        "clearing project content must not invalidate Engine assist resources");
    require(cache.create_animation("engine.test.idle") != nullptr,
        "clearing project content must not invalidate Engine assist animations");
    require(ui_animation.set_engine_animation(cache,"engine.test.idle"),
        "UiAnimation Engine Assist binding must survive project content cleanup");

    const std::filesystem::path missing_root = std::filesystem::temp_directory_path()
        / ("elysia_assist_cache_missing_"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto failed_reinitialize = cache.initialize(
        fixture.renderer(),
        elysia::assist::EngineAssistCatalog(missing_root),
        default_point_sizes);
    require(!failed_reinitialize.has_value(),
        "invalid Engine assist resources must reject initialization");
    require(cache.texture_count() == 6 && cache.font_count() == 35 && cache.locale_count() == 5
            && cache.animation_count() == 1,
        "a failed initialization must preserve the last complete cache transactionally");

    cache.shutdown();
    require(!cache.initialized() && cache.texture_count() == 0 && cache.font_count() == 0
            && cache.locale_count() == 0 && cache.animation_count() == 0,
        "cache shutdown must release all Engine-owned runtime resources");
    return 0;
}
