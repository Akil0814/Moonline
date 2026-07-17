#define SDL_MAIN_HANDLED

#include "engine/assist/engine_assist_cache.h"
#include "engine/assist/engine_assist_catalog.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/content_runtime_cleanup.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <chrono>
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

    const auto initialized = cache.initialize(fixture.renderer(), catalog);
    require(initialized.has_value(), "Engine assist cache must load all repository assist resources");
    require(cache.initialized(), "successful cache initialization must publish a live cache");
    require(cache.texture_count() == 5, "cache must own all five Engine textures");
    require(cache.font_count() == 35, "cache must own five font faces at seven fixed sizes");
    require(cache.locale_count() == 5, "cache must own all five Engine translation tables");
    require(cache.find_texture("engine.brand.elysia.white") != nullptr,
        "cache must expose the Engine startup logo by stable key");
    require(cache.find_font("zh-Hans", 30) != nullptr,
        "cache must expose Engine fonts by locale and point size");
    require(cache.find_translation("ja", "engine.settings.title") != nullptr,
        "cache must expose parsed Engine translations");
    require(elysia::assist::EngineAssistCache::map_project_locale("zh_cn") == "zh-Hans",
        "project Simplified Chinese must map to the Engine BCP-47 locale");

    elysia::loading::clear_loaded_content();
    require(cache.find_texture("engine.brand.elysia.white") != nullptr
            && cache.find_font("en", 20) != nullptr,
        "clearing project content must not invalidate Engine assist resources");

    const std::filesystem::path missing_root = std::filesystem::temp_directory_path()
        / ("elysia_assist_cache_missing_"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto failed_reinitialize = cache.initialize(
        fixture.renderer(),
        elysia::assist::EngineAssistCatalog(missing_root));
    require(!failed_reinitialize.has_value(),
        "invalid Engine assist resources must reject initialization");
    require(cache.texture_count() == 5 && cache.font_count() == 35 && cache.locale_count() == 5,
        "a failed initialization must preserve the last complete cache transactionally");

    cache.shutdown();
    require(!cache.initialized() && cache.texture_count() == 0 && cache.font_count() == 0
            && cache.locale_count() == 0,
        "cache shutdown must release all Engine-owned runtime resources");
    return 0;
}
