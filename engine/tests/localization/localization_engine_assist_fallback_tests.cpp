#define SDL_MAIN_HANDLED

#include "engine/assist/engine_assist_cache.h"
#include "engine/assist/engine_assist_catalog.h"
#include "engine/io/path/path_manager.h"
#include "engine/localization/localization_manager.h"
#include "engine/resources/resource_manager.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <filesystem>

namespace
{
using moonline::tests::require;

class LocalizationFixture
{
public:
    LocalizationFixture()
    {
        require(SDL_Init(SDL_INIT_VIDEO) == 0, "localization fallback tests must initialize SDL video");
        require((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == IMG_INIT_PNG,
            "localization fallback tests must initialize PNG support");
        require(TTF_Init() == 0, "localization fallback tests must initialize SDL_ttf");
        _surface = SDL_CreateRGBSurfaceWithFormat(0, 128, 128, 32, SDL_PIXELFORMAT_RGBA32);
        require(_surface != nullptr, "localization fallback tests must create a software surface");
        _renderer = SDL_CreateSoftwareRenderer(_surface);
        require(_renderer != nullptr, "localization fallback tests must create a software renderer");
    }

    ~LocalizationFixture()
    {
        elysia::localization::LocalizationManager::instance()->shutdown();
        elysia::resources::ResourceManager::instance()->clear();
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
    LocalizationFixture fixture;
    const std::filesystem::path source_root = MOONLINE_SOURCE_DIR;
    auto* path_manager = elysia::io::PathManager::instance();
    require(path_manager->init(source_root), "localization fallback tests must initialize project paths");

    elysia::assist::EngineAssistCache cache;
    require(cache.initialize(
        fixture.renderer(),
        elysia::assist::EngineAssistCatalog(source_root)).has_value(),
        "localization fallback tests must initialize Engine assist cache");

    auto* localization = elysia::localization::LocalizationManager::instance();
    require(localization->init(
        fixture.renderer(),
        source_root / "assets" / "configs" / "manifests" / "i18n_manifest.json",
        "en",
        &cache),
        "LocalizationManager must initialize with Engine assist defaults");
    require(localization->tr("common.save") == "Save",
        "project translations must remain the first lookup source");
    require(localization->tr("engine.settings.title") == "Settings",
        "missing Engine namespace keys must fall back to Engine translations");

    const elysia::localization::LocalizedTextStyle style{ .point_size = 20 };
    require(localization->get_text_texture("engine.settings.title", style) != nullptr,
        "Engine default font must render text before project content fonts load");

    auto* resources = elysia::resources::ResourceManager::instance();
    require(resources->load_font(
        "ui.latin.20",
        path_manager->fonts() / "fusion-pixel-10px-proportional-latin.ttf",
        20),
        "test must load a project font for precedence validation");
    int localized_width = 0;
    int localized_height = 0;
    require(localization->measure_raw_text("Moon", style, localized_width, localized_height),
        "localized measurement must succeed with a project font present");
    int engine_width = 0;
    int engine_height = 0;
    require(TTF_SizeUTF8(cache.find_font("en", 20), "Moon", &engine_width, &engine_height) == 0,
        "Engine font must measure the precedence probe text");
    require(localized_width == engine_width && localized_height == engine_height,
        "Engine fonts must take precedence over loaded project fonts");

    localization->shutdown();
    require(localization->init(
        fixture.renderer(),
        source_root / "assets" / "configs" / "manifests" / "i18n_manifest.json",
        "en"),
        "LocalizationManager must initialize without Engine assist cache");
    require(localization->measure_raw_text("Moon", style, localized_width, localized_height),
        "project font fallback must render when Engine cache is unavailable");
    int project_width = 0;
    int project_height = 0;
    require(TTF_SizeUTF8(resources->find_font("ui.latin.20"), "Moon", &project_width, &project_height) == 0,
        "project font must measure the fallback probe text");
    require(localized_width == project_width && localized_height == project_height,
        "project fonts must remain usable without Engine assist cache");

    localization->shutdown();
    cache.shutdown();
    return 0;
}
