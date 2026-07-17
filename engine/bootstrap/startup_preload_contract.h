#pragma once

#include <string_view>

namespace elysia::bootstrap::startup_preload
{
// Engine-owned startup branding lives in EngineAssistCache, outside the
// project preload manifest and its short-lived BootstrapTextureCache.
inline constexpr std::string_view EngineLogoTextureKey = "engine.brand.elysia.white";
}
