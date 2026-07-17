#include "engine/application/game_module.h"
#include "tests/support/test_assertions.h"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <span>
#include <type_traits>
#include <vector>

namespace
{
using elysia::application::ApplicationDescriptor;
using elysia::application::ApplicationEngineLogoVariant;
using elysia::application::ApplicationFontSource;
using elysia::application::ApplicationScaleStrategy;
using elysia::application::ApplicationTextureFilter;
using elysia::application::ApplicationTypographyProfile;
using elysia::application::resolve_application_font_settings;
using elysia::ui::UiTypographyRole;
using moonline::tests::require;

static_assert(
    !std::is_default_constructible_v<ApplicationTypographyProfile>,
    "typography profiles must provide a complete role table");

void require_sizes(
    std::span<const int> actual,
    std::initializer_list<int> expected,
    const char* message)
{
    require(
        std::vector<int>(actual.begin(),actual.end())
            == std::vector<int>(expected),
        message);
}

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
        descriptor.presentation.fonts.ui.source
                == ApplicationFontSource::EngineBuiltIn
            && descriptor.presentation.fonts.floating_number.source
                == ApplicationFontSource::EngineBuiltIn,
        "application presentation must use engine fonts by default");
    require(
        !descriptor.presentation.fonts.ui.typography_override
            && !descriptor.presentation.fonts.floating_number.point_size_override,
        "application presentation must leave font size overrides empty by default");

    const auto resolved =
        resolve_application_font_settings(descriptor.presentation.fonts);
    require(resolved.has_value(),
        "default application font settings must resolve");
    require(
        resolved->ui_typography().point_sizes()
            == ApplicationTypographyProfile::PointSizes{
                30,30,70,50,30,20,30,20,30,60,30,20,30,30,30,10,40
            },
        "unconfigured UI typography must resolve to Engine defaults");
    require(
        resolved->floating_number_point_size() == 20,
        "unconfigured floating numbers must resolve to 20pt");
    require_sizes(
        resolved->engine_point_sizes(),
        {10,20,30,40,50,60,70},
        "default Engine point sizes must be sorted and deduplicated");
    require(
        resolved->project_point_sizes().empty(),
        "Engine-only defaults must not request project font sizes");
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

void require_custom_font_settings_resolve()
{
    ApplicationTypographyProfile::PointSizes point_sizes{};
    point_sizes.fill(24);

    ApplicationDescriptor descriptor;
    descriptor.presentation.fonts.ui.source = ApplicationFontSource::Project;
    descriptor.presentation.fonts.ui.typography_override =
        ApplicationTypographyProfile(point_sizes);
    descriptor.presentation.fonts.floating_number.source =
        ApplicationFontSource::Project;
    descriptor.presentation.fonts.floating_number.point_size_override = 18;

    const auto resolved =
        resolve_application_font_settings(descriptor.presentation.fonts);
    require(resolved.has_value(),
        "custom application font settings must resolve");
    require(
        resolved->ui_source() == ApplicationFontSource::Project
            && resolved->floating_number_source()
                == ApplicationFontSource::Project,
        "resolved settings must preserve configured font sources");
    require(
        resolved->ui_typography().point_sizes() == point_sizes
            && resolved->floating_number_point_size() == 18,
        "resolved settings must preserve explicit size overrides");
    require_sizes(
        resolved->engine_point_sizes(),
        {18,24},
        "Engine fallback sizes must include every resolved font size");
    require_sizes(
        resolved->project_point_sizes(),
        {18,24},
        "project sizes must include only requested project font sizes");

    descriptor.presentation.fonts.ui.source =
        ApplicationFontSource::EngineBuiltIn;
    const auto mixed =
        resolve_application_font_settings(descriptor.presentation.fonts);
    require(mixed.has_value(),
        "mixed Engine and project font settings must resolve");
    require_sizes(
        mixed->engine_point_sizes(),
        {18,24},
        "Engine sizes must remain independent of configured sources");
    require_sizes(
        mixed->project_point_sizes(),
        {18},
        "project sizes must omit Engine-only UI sizes");
}

void require_invalid_font_settings_fail()
{
    ApplicationDescriptor descriptor;
    ApplicationTypographyProfile::PointSizes point_sizes{};
    point_sizes.fill(20);
    point_sizes.front() = 0;
    descriptor.presentation.fonts.ui.typography_override =
        ApplicationTypographyProfile(point_sizes);
    require(
        !resolve_application_font_settings(
            descriptor.presentation.fonts),
        "non-positive UI typography must fail resolution");

    descriptor.presentation.fonts.ui.typography_override.reset();
    descriptor.presentation.fonts.floating_number.point_size_override = 0;
    require(
        !resolve_application_font_settings(
            descriptor.presentation.fonts),
        "non-positive floating-number typography must fail resolution");

    descriptor.presentation.fonts.floating_number.point_size_override.reset();
    descriptor.presentation.fonts.ui.source =
        static_cast<ApplicationFontSource>(255);
    require(
        !resolve_application_font_settings(
            descriptor.presentation.fonts),
        "unknown font sources must fail resolution");
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
    require_custom_font_settings_resolve();
    require_invalid_font_settings_fail();
    require_engine_logo_variants_are_data_only();
    return EXIT_SUCCESS;
}
