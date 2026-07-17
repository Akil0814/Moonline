#pragma once

#include "../ui/text/ui_typography.h"

#include <array>
#include <cstddef>
#include <utility>

namespace elysia::application
{
enum class ApplicationScaleStrategy
{
    PixelPerfect,
    AspectFit
};

enum class ApplicationTextureFilter
{
    Nearest,
    Linear
};

struct ApplicationRenderSettings
{
    ApplicationScaleStrategy scale_strategy =
        ApplicationScaleStrategy::PixelPerfect;
    ApplicationTextureFilter texture_filter =
        ApplicationTextureFilter::Nearest;
};

class ApplicationTypographyProfile
{
public:
    static constexpr std::size_t RoleCount =
        static_cast<std::size_t>(elysia::ui::UiTypographyRole::Count);
    using PointSizes = std::array<int, RoleCount>;

    ApplicationTypographyProfile() = delete;

    explicit constexpr ApplicationTypographyProfile(
        PointSizes point_sizes) noexcept
        : _point_sizes(std::move(point_sizes))
    {
    }

    [[nodiscard]] constexpr int point_size(
        elysia::ui::UiTypographyRole role) const
    {
        return _point_sizes.at(static_cast<std::size_t>(role));
    }

    [[nodiscard]] constexpr const PointSizes& point_sizes() const noexcept
    {
        return _point_sizes;
    }

    [[nodiscard]] static constexpr ApplicationTypographyProfile
        engine_defaults() noexcept
    {
        return ApplicationTypographyProfile(PointSizes{
            30, // Label
            30, // LabelMuted
            70, // Title
            50, // Subtitle
            30, // Button
            20, // ButtonCompact
            30, // Input
            20, // InputPlaceholder
            30, // Number
            60, // DialogTitle
            30, // DialogBody
            20, // DialogAction
            30, // SliderValue
            30, // CheckboxLabel
            30, // RadioLabel
            10, // Caption
            40  // Heading
        });
    }

private:
    PointSizes _point_sizes;
};

enum class ApplicationFontSource
{
    EngineBuiltIn,
    Project
};

struct ApplicationUiFontSettings
{
    ApplicationFontSource source = ApplicationFontSource::EngineBuiltIn;
    ApplicationTypographyProfile typography =
        ApplicationTypographyProfile::engine_defaults();
};

struct ApplicationFloatingNumberFontSettings
{
    ApplicationFontSource source = ApplicationFontSource::EngineBuiltIn;
    int point_size = 20;
};

struct ApplicationFontSettings
{
    ApplicationUiFontSettings ui;
    ApplicationFloatingNumberFontSettings floating_number;
};

enum class ApplicationEngineLogoVariant
{
    Default,
    Black,
    BlackAlphaInverse,
    LightEdge,
    White
};

struct ApplicationStartupPresentationSettings
{
    ApplicationEngineLogoVariant engine_logo =
        ApplicationEngineLogoVariant::White;
};

struct ApplicationPresentationSettings
{
    ApplicationRenderSettings render;
    ApplicationFontSettings fonts;
    ApplicationStartupPresentationSettings startup;
};
}
