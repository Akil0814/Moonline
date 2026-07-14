#define SDL_MAIN_HANDLED

#include "engine/effects/effect_manager.h"
#include "engine/io/path/path_manager.h"
#include "engine/localization/localization_manager.h"
#include "engine/resources/resource_manager.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>
#include <cstdlib>
#include <iostream>

namespace
{
using moonline::tests::require;

struct EffectDigitCacheFixture
{
    EffectDigitCacheFixture()
    {
        require(SDL_Init(SDL_INIT_VIDEO) == 0, "effect digit cache tests must initialize SDL video");
        require(TTF_Init() == 0, "effect digit cache tests must initialize SDL_ttf");
        surface = SDL_CreateRGBSurfaceWithFormat(0, 64, 64, 32, SDL_PIXELFORMAT_RGBA32);
        require(surface != nullptr, "effect digit cache tests must create a surface");
        renderer = SDL_CreateSoftwareRenderer(surface);
        require(renderer != nullptr, "effect digit cache tests must create a software renderer");
    }

    ~EffectDigitCacheFixture()
    {
        elysia::effects::EffectManager::instance()->reset_digit_caches();
        elysia::localization::LocalizationManager::instance()->shutdown();
        elysia::resources::ResourceManager::instance()->clear();
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(surface);
        TTF_Quit();
        SDL_Quit();
    }

    SDL_Surface* surface = nullptr;
    SDL_Renderer* renderer = nullptr;
};
}

void test_digit_cache_initialization_reuse_and_reset(EffectDigitCacheFixture& fixture)
{
    using namespace elysia;

    SDL_Renderer* renderer = fixture.renderer;

    effects::EffectManager* effect_manager = effects::EffectManager::instance();
    localization::LocalizationManager* localization_manager = localization::LocalizationManager::instance();
    resources::ResourceManager* resource_manager = resources::ResourceManager::instance();
    io::PathManager* path_manager = io::PathManager::instance();

    effect_manager->reset_digit_caches();
    localization_manager->shutdown();
    require(effect_manager->digit_cache(effects::EffectDigitColor::White) == nullptr,
        "digit cache must be unavailable without a renderer");

    require(path_manager->init(), "effect digit cache tests must initialize the project path manager");
    resource_manager->clear();
    require(localization_manager->init(
        renderer,
        path_manager->configs() / "manifests" / "i18n_manifest.json",
        "en"), "effect digit cache tests must initialize localization");
    number::DigitCache* missing_font_cache = effect_manager->digit_cache(effects::EffectDigitColor::White);
    require(missing_font_cache != nullptr && missing_font_cache->get_glyph('8') == nullptr,
        "digit cache must safely reject glyph generation when the base font is unavailable");
    require(resource_manager->load_font(
        "ui.latin.20",
        path_manager->fonts() / "fusion-pixel-10px-proportional-latin.ttf",
        20), "effect digit cache tests must load the base digit font");

    constexpr std::array colors{
        effects::EffectDigitColor::White,
        effects::EffectDigitColor::Black,
        effects::EffectDigitColor::Yellow,
        effects::EffectDigitColor::Green,
        effects::EffectDigitColor::Red,
        effects::EffectDigitColor::Blue,
        effects::EffectDigitColor::LightBlue,
        effects::EffectDigitColor::Orange,
        effects::EffectDigitColor::Purple
    };

    number::DigitCache* white_cache = effect_manager->digit_cache(effects::EffectDigitColor::White);
    require(white_cache != nullptr && white_cache->source_mode() == number::DigitSourceMode::Font,
        "white digit cache must configure a font source");
    SDL_Texture* first_white_glyph = white_cache->get_glyph('8');
    require(first_white_glyph != nullptr && white_cache->get_glyph('8') == first_white_glyph,
        "the same color cache must reuse generated glyph textures");
    require(white_cache->get_glyph('x') == nullptr, "unsupported glyphs must fail safely");

    for (const effects::EffectDigitColor color : colors)
    {
        number::DigitCache* cache = effect_manager->digit_cache(color);
        require(cache != nullptr && cache->get_glyph('5') != nullptr,
            "each configured effect digit color must generate glyphs");
    }
    require(effect_manager->digit_cache(effects::EffectDigitColor::White)
            != effect_manager->digit_cache(effects::EffectDigitColor::Yellow),
        "different effect digit colors must use independent caches");

    effect_manager->reset_digit_caches();
    require(white_cache->source_mode() == number::DigitSourceMode::None,
        "reset must release cached font glyph sources");
    number::DigitCache* rebuilt_white_cache = effect_manager->digit_cache(effects::EffectDigitColor::White);
    require(rebuilt_white_cache != nullptr && rebuilt_white_cache->get_glyph('8') != nullptr,
        "digit caches must rebuild lazily after reset");

    effect_manager->reset_digit_caches();
    localization_manager->shutdown();
    require(effect_manager->digit_cache(effects::EffectDigitColor::White) == nullptr,
        "digit cache must safely reject requests after renderer shutdown");
    resource_manager->clear();
}

int main()
{
    EffectDigitCacheFixture fixture;
    test_digit_cache_initialization_reuse_and_reset(fixture);
    return EXIT_SUCCESS;
}
