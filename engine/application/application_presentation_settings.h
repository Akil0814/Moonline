#pragma once

#include "../ui/text/ui_typography.h"

#include <array>
#include <cstddef>
#include <optional>
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

private:
    PointSizes _point_sizes;
};

struct ApplicationProjectFontReplacement
{
    ApplicationTypographyProfile typography;
};

struct ApplicationFontSettings
{
    std::optional<ApplicationProjectFontReplacement>
        project_font_replacement = std::nullopt;
};

struct ApplicationPresentationSettings
{
    ApplicationRenderSettings render;
    ApplicationFontSettings fonts;
};
}
