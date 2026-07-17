#pragma once

#include "../ui/text/ui_typography.h"

#include <array>
#include <cstddef>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace elysia::application
{
enum class ApplicationTextureFilter
{
    Nearest,
    Linear
};

struct ApplicationRenderSettings
{
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
    std::optional<ApplicationTypographyProfile> typography_override;
};

struct ApplicationFloatingNumberFontSettings
{
    ApplicationFontSource source = ApplicationFontSource::EngineBuiltIn;
    std::optional<int> point_size_override;
};

struct ApplicationFontSettings
{
    ApplicationUiFontSettings ui;
    ApplicationFloatingNumberFontSettings floating_number;
};

class ResolvedApplicationFontSettings
{
public:
    static constexpr int DefaultFloatingNumberPointSize = 20;

    [[nodiscard]] ApplicationFontSource ui_source() const noexcept;
    [[nodiscard]] const ApplicationTypographyProfile&
        ui_typography() const noexcept;
    [[nodiscard]] ApplicationFontSource
        floating_number_source() const noexcept;
    [[nodiscard]] int floating_number_point_size() const noexcept;
    [[nodiscard]] std::span<const int> engine_point_sizes() const noexcept;
    [[nodiscard]] std::span<const int> project_point_sizes() const noexcept;

private:
    friend std::expected<ResolvedApplicationFontSettings, std::string>
        resolve_application_font_settings(const ApplicationFontSettings&);

    ResolvedApplicationFontSettings(
        ApplicationFontSource ui_source,
        ApplicationTypographyProfile ui_typography,
        ApplicationFontSource floating_number_source,
        int floating_number_point_size,
        std::vector<int> engine_point_sizes,
        std::vector<int> project_point_sizes);

private:
    ApplicationFontSource _ui_source;
    ApplicationTypographyProfile _ui_typography;
    ApplicationFontSource _floating_number_source;
    int _floating_number_point_size;
    std::vector<int> _engine_point_sizes;
    std::vector<int> _project_point_sizes;
};

[[nodiscard]] std::expected<ResolvedApplicationFontSettings, std::string>
    resolve_application_font_settings(const ApplicationFontSettings& settings);

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
