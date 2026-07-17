#pragma once

#include <cstdint>
#include <limits>

namespace elysia::scene
{
using SceneKey = std::uint32_t;

namespace SceneKeys
{
inline constexpr SceneKey Invalid = 0;

inline constexpr SceneKey GameBegin = 1;
inline constexpr SceneKey GameEnd = 999;
inline constexpr SceneKey ElysiaEasterEgg = 1111;

// This value is a range marker, not a usable scene key. Engine-owned
// scenes occupy the values strictly above it.
inline constexpr SceneKey EngineMarker = 0xFFFF0000u;
inline constexpr SceneKey EngineBegin = EngineMarker + 1u;
inline constexpr SceneKey EngineEnd = std::numeric_limits<SceneKey>::max();

[[nodiscard]] constexpr bool is_game(SceneKey key) noexcept
{
    return key >= GameBegin && key <= GameEnd;
}

[[nodiscard]] constexpr bool is_engine(SceneKey key) noexcept
{
    return key >= EngineBegin;
}

[[nodiscard]] constexpr bool is_easter_egg(SceneKey key) noexcept
{
    return key == ElysiaEasterEgg;
}

[[nodiscard]] constexpr bool is_supported(SceneKey key) noexcept
{
    return is_game(key) || is_engine(key) || is_easter_egg(key);
}

[[nodiscard]] constexpr bool is_reserved(SceneKey key) noexcept
{
    return key != Invalid && !is_supported(key);
}
}

namespace builtin
{
inline constexpr SceneKey StartupLoading = 0xFFFF0001u;
inline constexpr SceneKey Settings = 0xFFFF0002u;
inline constexpr SceneKey StartupFailure = 0xFFFF0005u;
}

}
