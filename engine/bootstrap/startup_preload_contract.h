#pragma once

#include <string_view>

namespace elysia::bootstrap::startup_preload
{
// Engine-owned startup branding is always loaded independently of the
// project's preload manifest.
inline constexpr std::string_view EngineLogoTextureKey = "elysia.brand.logo";
inline constexpr std::string_view EngineLogoAssetPath = "engine/preload/elysia_logo.png";
}
