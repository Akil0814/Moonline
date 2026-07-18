#define SDL_MAIN_HANDLED

#include "engine/assist/engine_assist_keys.h"
#include "engine/bootstrap/bootstrap_texture_cache.h"
#include "engine/bootstrap/bootstrapper.h"
#include "engine/bootstrap/startup_preload_loader.h"
#include "engine/config/user_config_service.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/content_runtime_cleanup.h"
#include "engine/resources/resource_manager.h"
#include "engine/scene/builtin/startup_loading_scene.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_image.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <utility>

namespace
{
using moonline::tests::require;

elysia::resources::TexturePtr make_texture(SDL_Renderer* renderer)
{
    return elysia::resources::TexturePtr(SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STATIC,
        8,
        8
    ));
}

void test_bootstrap_texture_cache_and_preload_lifetime()
{
    using namespace elysia;
    require(SDL_Init(SDL_INIT_VIDEO) == 0,"bootstrap texture tests must initialize SDL video");
    require((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == IMG_INIT_PNG,
        "bootstrap texture tests must initialize PNG support");

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0,128,128,32,SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* second_surface = SDL_CreateRGBSurfaceWithFormat(0,128,128,32,SDL_PIXELFORMAT_RGBA32);
    require(surface && second_surface,"bootstrap texture tests must create software surfaces");
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    SDL_Renderer* second_renderer = SDL_CreateSoftwareRenderer(second_surface);
    require(renderer && second_renderer,"bootstrap texture tests must create software renderers");

    {
        bootstrap::BootstrapTextureCache cache;
        resources::TexturePtr first = make_texture(renderer);
        SDL_Texture* first_raw = first.get();
        require(first_raw && cache.store("logo",std::move(first)),
            "bootstrap cache must accept a valid texture");
        require(cache.find("logo") == first_raw,
            "bootstrap cache must retain and reuse stored textures");
        require(cache.find("missing") == nullptr,"unknown bootstrap keys must return null");
        require(!cache.store("logo",make_texture(renderer))
                && cache.find("logo") == first_raw,
            "duplicate bootstrap keys must be rejected without replacing the original texture");
        cache.clear();
        require(cache.find("logo") == nullptr,
            "clearing the bootstrap cache must release all entries");

        auto* paths = io::PathManager::instance();
        auto* resource_manager = resources::ResourceManager::instance();
        require(paths->init(),"bootstrap texture tests must initialize project paths");
        resource_manager->clear();
        const std::size_t resource_count_before = resource_manager->resource_count();

        bootstrap::StartupPreloadLoader loader;
        loader.set_manifest_path(paths->configs() / "manifests" / "preload_manifest.json");
        require(
            loader.load(renderer).has_value(),
            "startup loader must load the real preload manifest");
        SDL_Texture* logo = loader.find_texture("moonline.brand.logo");
        require(logo != nullptr,"startup loader must expose the keyed project logo");
        require(
            loader.load(renderer).has_value()
                && loader.find_texture("moonline.brand.logo") == logo,
            "startup preload must be idempotent for the same renderer");
        require(
            loader.find_texture(assist::asset_keys::ElysiaWhiteTexture) == nullptr,
            "startup loader must not own Engine Assist textures");
        require(resource_manager->resource_count() == resource_count_before,
            "bootstrap preload must not publish textures to ResourceManager");

        loading::clear_loaded_content();
        int width = 0;
        int height = 0;
        require(loader.find_texture("moonline.brand.logo") == logo
            && SDL_QueryTexture(logo,nullptr,nullptr,&width,&height) == 0
            && width > 0
            && height > 0,
            "clearing game content must not invalidate bootstrap textures");

        require(!loader.load(nullptr),"a reload with a missing renderer must fail");
        require(loader.find_texture("moonline.brand.logo") == logo,
            "failed reloads must preserve the last complete bootstrap cache");

        require(
            loader.load(second_renderer).has_value(),
            "renderer changes must rebuild bootstrap textures");
        SDL_Texture* rebuilt_logo = loader.find_texture("moonline.brand.logo");
        require(rebuilt_logo && SDL_QueryTexture(rebuilt_logo,nullptr,nullptr,&width,&height) == 0,
            "rebuilt bootstrap textures must belong to a usable renderer cache");

        loader.release_textures();
        require(loader.find_texture("moonline.brand.logo") == nullptr,
            "released bootstrap textures must no longer be queryable");
        require(
            loader.load(renderer).has_value()
                && loader.find_texture("moonline.brand.logo"),
            "release must retain manifest configuration for a later reload");

        const std::filesystem::path invalid_manifest =
            std::filesystem::temp_directory_path() / "moonline_invalid_preload_manifest.json";
        {
            std::ofstream output(invalid_manifest,std::ios::trunc);
            output << R"({"textures":[{"key":"project.valid","file":"Akil_icon_1024.png"},{"key":"project.invalid"}]})";
        }
        bootstrap::StartupPreloadLoader failing_loader;
        failing_loader.set_manifest_path(invalid_manifest);
        require(!failing_loader.load(renderer),"a malformed keyed preload manifest must fail");
        require(failing_loader.find_texture("project.valid") == nullptr,
            "partial preload results must never become visible");
        std::error_code remove_error;
        std::filesystem::remove(invalid_manifest,remove_error);

        const std::filesystem::path missing_manifest =
            std::filesystem::temp_directory_path() / "moonline_optional_preload_manifest.json";
        {
            std::ofstream output(missing_manifest,std::ios::trunc);
            output << R"({"textures":[{"key":"project.missing","file":"missing-bootstrap-texture.png"}]})";
        }
        bootstrap::StartupPreloadLoader missing_loader;
        missing_loader.set_manifest_path(missing_manifest);
        require(
            !missing_loader.load(renderer),
            "a missing required project texture must fail startup preload");
        require(missing_loader.find_texture("project.missing") == nullptr,
            "a failed required preload must not publish a texture");
        require(missing_loader.find_texture(
            assist::asset_keys::ElysiaWhiteTexture) == nullptr,
            "project startup preload must not expose Engine Assist textures");
        std::filesystem::remove(missing_manifest,remove_error);

        const std::filesystem::path invalid_texture =
            paths->preload() / "moonline_invalid_preload_texture_test.png";
        const std::filesystem::path undecodable_manifest =
            std::filesystem::temp_directory_path()
            / "moonline_undecodable_preload_manifest.json";
        {
            std::ofstream invalid_output(invalid_texture,std::ios::trunc);
            invalid_output << "not an image";
            std::ofstream manifest_output(undecodable_manifest,std::ios::trunc);
            manifest_output
                << R"({"textures":[{"key":"project.invalid_image","file":")"
                << invalid_texture.filename().string()
                << R"("}]})";
        }
        bootstrap::StartupPreloadLoader undecodable_loader;
        undecodable_loader.set_manifest_path(undecodable_manifest);
        require(
            !undecodable_loader.load(renderer),
            "an undecodable required texture must fail startup preload");
        require(
            undecodable_loader.find_texture("project.invalid_image") == nullptr,
            "an undecodable texture must not publish a partial cache");
        std::filesystem::remove(undecodable_manifest,remove_error);
        std::filesystem::remove(invalid_texture,remove_error);

        const std::filesystem::path reserved_manifest =
            std::filesystem::temp_directory_path()
            / "moonline_reserved_preload_manifest.json";
        {
            std::ofstream output(reserved_manifest,std::ios::trunc);
            output
                << R"({"textures":[{"key":")"
                << assist::asset_keys::ElysiaWhiteTexture
                << R"(","file":"Akil_icon_1024.png"}]})";
        }
        bootstrap::StartupPreloadLoader reserved_loader;
        reserved_loader.set_manifest_path(reserved_manifest);
        require(
            !reserved_loader.load(renderer),
            "project preload must reject the Engine Assist logo key");
        std::filesystem::remove(reserved_manifest,remove_error);

        loader.release_textures();
        missing_loader.release_textures();
        undecodable_loader.release_textures();
        reserved_loader.release_textures();
        resource_manager->clear();

        const std::filesystem::path source_root = paths->root();
        const std::filesystem::path lifecycle_root =
            std::filesystem::temp_directory_path()
            / "moonline_bootstrap_lifecycle_tests";
        std::filesystem::remove_all(lifecycle_root,remove_error);
        for (const char* directory : {
                "assets/audio",
                "assets/textures",
                "assets/fonts",
                "assets/preload" })
        {
            std::filesystem::create_directories(
                lifecycle_root / directory);
        }
        std::filesystem::copy_file(
            source_root / "assets/content_registry.json",
            lifecycle_root / "assets/content_registry.json");
        std::filesystem::copy(
            source_root / "assets/configs",
            lifecycle_root / "assets/configs",
            std::filesystem::copy_options::recursive);
        std::filesystem::copy_file(
            source_root / "assets/preload/Akil_icon_1024.png",
            lifecycle_root / "assets/preload/Akil_icon_1024.png");

        auto* bootstrapper = bootstrap::Bootstrapper::instance();
        const auto parsed =
            bootstrapper->parse_runtime_settings(lifecycle_root);
        require(parsed.has_value(),
            "bootstrap lifecycle test must parse runtime settings");
        require(
            bootstrapper->preload_startup_resources(renderer).has_value()
                && bootstrapper->find_preload_texture(
                    "moonline.brand.logo") != nullptr,
            "bootstrap lifecycle test must preload the project logo");

        scene::builtin::StartupLoadingScene startup_scene;
        startup_scene.on_exit();
        require(
            bootstrapper->find_preload_texture("moonline.brand.logo")
                == nullptr,
            "leaving StartupLoadingScene must release preload textures");
        config::UserConfigService::instance()->shutdown();
        std::filesystem::remove_all(lifecycle_root,remove_error);
    }

    SDL_DestroyRenderer(second_renderer);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(second_surface);
    SDL_FreeSurface(surface);
    IMG_Quit();
    SDL_Quit();
}
}

int main()
{
    test_bootstrap_texture_cache_and_preload_lifetime();
    return EXIT_SUCCESS;
}
