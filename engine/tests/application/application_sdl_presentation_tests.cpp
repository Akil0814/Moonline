#define SDL_MAIN_HANDLED

#include "engine/application/application_sdl_presentation.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>

#include <cstdlib>
#include <string>

namespace
{
using elysia::application::ApplicationRenderSettings;
using elysia::application::ApplicationScaleStrategy;
using elysia::application::ApplicationTextureFilter;
using elysia::application::detail::configure_sdl_render_hints;
using elysia::application::detail::configure_sdl_renderer_presentation;
using moonline::tests::require;

class SdlPresentationFixture
{
public:
    SdlPresentationFixture()
    {
        require(
            SDL_Init(SDL_INIT_VIDEO) == 0,
            "SDL presentation tests must initialize SDL video");

        _surface = SDL_CreateRGBSurfaceWithFormat(
            0,
            1280,
            720,
            32,
            SDL_PIXELFORMAT_RGBA32);
        require(
            _surface != nullptr,
            "SDL presentation tests must create a software surface");

        _renderer = SDL_CreateSoftwareRenderer(_surface);
        require(
            _renderer != nullptr,
            "SDL presentation tests must create a software renderer");
    }

    ~SdlPresentationFixture()
    {
        SDL_DestroyRenderer(_renderer);
        SDL_FreeSurface(_surface);
        SDL_Quit();
    }

    [[nodiscard]] SDL_Renderer* renderer() const noexcept
    {
        return _renderer;
    }

private:
    SDL_Surface* _surface = nullptr;
    SDL_Renderer* _renderer = nullptr;
};

void require_scale_strategy(
    SDL_Renderer* renderer,
    ApplicationScaleStrategy strategy,
    SDL_bool expected_integer_scale)
{
    const ApplicationRenderSettings settings{
        .scale_strategy = strategy,
        .texture_filter = ApplicationTextureFilter::Nearest
    };
    const auto result = configure_sdl_renderer_presentation(
        renderer,
        640,
        360,
        settings);
    require(result.has_value(), "SDL renderer presentation must be applied");

    int logical_width = 0;
    int logical_height = 0;
    SDL_RenderGetLogicalSize(renderer,&logical_width,&logical_height);
    require(
        logical_width == 640 && logical_height == 360,
        "SDL renderer presentation must retain the logical size");
    require(
        SDL_RenderGetIntegerScale(renderer) == expected_integer_scale,
        "SDL renderer presentation must apply the requested integer scaling");
}

void require_texture_filter(
    SDL_Renderer* renderer,
    ApplicationTextureFilter filter,
    SDL_ScaleMode expected_mode)
{
    const ApplicationRenderSettings settings{
        .scale_strategy = ApplicationScaleStrategy::AspectFit,
        .texture_filter = filter
    };
    const auto hint_result = configure_sdl_render_hints(settings);
    require(hint_result.has_value(), "SDL render hints must be applied");

    const char* logical_size_mode =
        SDL_GetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE);
    require(
        logical_size_mode
            && std::string(logical_size_mode) == "letterbox",
        "SDL logical size mode must be letterbox");

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        8,
        8);
    require(texture != nullptr, "SDL presentation tests must create a texture");

    SDL_ScaleMode actual_mode = SDL_ScaleModeNearest;
    require(
        SDL_GetTextureScaleMode(texture,&actual_mode) == 0,
        "SDL presentation tests must query texture scale mode");
    require(
        actual_mode == expected_mode,
        "new SDL textures must inherit the requested scale mode");

    SDL_DestroyTexture(texture);
}

void require_invalid_settings_fail(SDL_Renderer* renderer)
{
    const ApplicationRenderSettings invalid_filter{
        .scale_strategy = ApplicationScaleStrategy::PixelPerfect,
        .texture_filter = static_cast<ApplicationTextureFilter>(255)
    };
    require(
        !configure_sdl_render_hints(invalid_filter).has_value(),
        "unknown texture filters must fail SDL hint configuration");

    const ApplicationRenderSettings invalid_strategy{
        .scale_strategy = static_cast<ApplicationScaleStrategy>(255),
        .texture_filter = ApplicationTextureFilter::Nearest
    };
    require(
        !configure_sdl_render_hints(invalid_strategy).has_value(),
        "unknown scale strategies must fail SDL hint configuration");
    require(
        !configure_sdl_renderer_presentation(
            renderer,
            640,
            360,
            invalid_strategy).has_value(),
        "unknown scale strategies must fail renderer configuration");
}
}

int main()
{
    SdlPresentationFixture fixture;

    require_scale_strategy(
        fixture.renderer(),
        ApplicationScaleStrategy::PixelPerfect,
        SDL_TRUE);
    require_scale_strategy(
        fixture.renderer(),
        ApplicationScaleStrategy::AspectFit,
        SDL_FALSE);

    require_texture_filter(
        fixture.renderer(),
        ApplicationTextureFilter::Nearest,
        SDL_ScaleModeNearest);
    require_texture_filter(
        fixture.renderer(),
        ApplicationTextureFilter::Linear,
        SDL_ScaleModeLinear);

    require_invalid_settings_fail(fixture.renderer());
    return EXIT_SUCCESS;
}
