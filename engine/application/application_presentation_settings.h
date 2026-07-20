#pragma once

#include "../typography/font_settings.h"

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
    elysia::typography::FontSettings fonts;
    ApplicationStartupPresentationSettings startup;
};
}
