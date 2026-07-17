#include "engine/application/game_module.h"
#include "tests/support/test_assertions.h"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace
{
using elysia::application::ApplicationDescriptor;
using elysia::application::ApplicationEngineLogoVariant;
using elysia::application::ApplicationProjectFontReplacement;
using elysia::application::ApplicationScaleStrategy;
using elysia::application::ApplicationTextureFilter;
using elysia::application::ApplicationTypographyProfile;
using elysia::ui::UiTypographyRole;
using moonline::tests::require;

static_assert(
    !std::is_default_constructible_v<ApplicationTypographyProfile>,
    "typography profiles must provide a complete role table");

void require_default_presentation_settings()
{
    const ApplicationDescriptor descriptor;

    require(
        descriptor.presentation.render.scale_strategy
            == ApplicationScaleStrategy::PixelPerfect,
        "application presentation must default to pixel-perfect scaling");
    require(
        descriptor.presentation.render.texture_filter
            == ApplicationTextureFilter::Nearest,
        "application presentation must default to nearest filtering");
    require(
        !descriptor.presentation.fonts.project_font_replacement.has_value(),
        "application presentation must use engine fonts by default");
    require(
        descriptor.presentation.startup.engine_logo
            == ApplicationEngineLogoVariant::White,
        "application startup presentation must default to the white engine logo");
}

void require_render_settings_are_data_only()
{
    ApplicationDescriptor descriptor;
    descriptor.presentation.render.scale_strategy =
        ApplicationScaleStrategy::AspectFit;
    descriptor.presentation.render.texture_filter =
        ApplicationTextureFilter::Linear;

    require(
        descriptor.presentation.render.scale_strategy
            == ApplicationScaleStrategy::AspectFit,
        "application presentation must retain the selected scale strategy");
    require(
        descriptor.presentation.render.texture_filter
            == ApplicationTextureFilter::Linear,
        "application presentation must retain the selected texture filter");
}

void require_complete_typography_profile()
{
    ApplicationTypographyProfile::PointSizes point_sizes{};
    for (std::size_t index = 0;index < point_sizes.size();++index)
        point_sizes[index] = static_cast<int>((index + 1) * 10);

    const ApplicationTypographyProfile profile(point_sizes);

    require(
        point_sizes.size()
            == static_cast<std::size_t>(UiTypographyRole::Count),
        "typography profile must contain one point size for every role");

    for (std::size_t index = 0;index < point_sizes.size();++index)
    {
        require(
            profile.point_size(static_cast<UiTypographyRole>(index))
                == point_sizes[index],
            "typography profile must retain each role point size");
    }

    ApplicationDescriptor descriptor;
    descriptor.presentation.fonts.project_font_replacement =
        ApplicationProjectFontReplacement{ .typography = profile };

    require(
        descriptor.presentation.fonts.project_font_replacement.has_value(),
        "project font replacement must be distinguishable from engine fonts");
    require(
        descriptor.presentation.fonts.project_font_replacement
                ->typography.point_sizes()
            == point_sizes,
        "project font replacement must retain the complete typography profile");
}

void require_engine_logo_variants_are_data_only()
{
    constexpr std::array variants{
        ApplicationEngineLogoVariant::Default,
        ApplicationEngineLogoVariant::Black,
        ApplicationEngineLogoVariant::BlackAlphaInverse,
        ApplicationEngineLogoVariant::LightEdge,
        ApplicationEngineLogoVariant::White
    };

    ApplicationDescriptor descriptor;
    for (const ApplicationEngineLogoVariant variant : variants)
    {
        descriptor.presentation.startup.engine_logo = variant;
        require(
            descriptor.presentation.startup.engine_logo == variant,
            "application presentation must retain the selected engine logo variant");
    }
}
}

int main()
{
    require_default_presentation_settings();
    require_render_settings_are_data_only();
    require_complete_typography_profile();
    require_engine_logo_variants_are_data_only();
    return EXIT_SUCCESS;
}
