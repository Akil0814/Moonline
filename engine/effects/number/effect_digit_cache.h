#pragma once

#include "../../number/digit_cache.h"

#include <SDL.h>

#include <array>
#include <cstddef>

namespace elysia::effects
{
enum class EffectDigitColor : std::size_t
{
    White,
    Black,
    Yellow,
    Green,
    Red,
    Blue,
    LightBlue,
    Orange,
    Purple,
    Count
};

class EffectDigitCache
{
public:
    static constexpr int k_point_size = 20;
    static constexpr std::size_t k_color_count = static_cast<std::size_t>(EffectDigitColor::Count);

    [[nodiscard]] elysia::number::DigitCache* digit_cache(EffectDigitColor color);
    void reset() noexcept;

private:
    [[nodiscard]] static constexpr std::size_t color_index(EffectDigitColor color) noexcept
    {
        return static_cast<std::size_t>(color);
    }

    [[nodiscard]] static elysia::core::Color color_value(EffectDigitColor color) noexcept;
    void reset_for_renderer(SDL_Renderer* renderer) noexcept;

private:
    std::array<elysia::number::DigitCache, k_color_count> _digit_caches;
    std::array<bool, k_color_count> _configured{};
    SDL_Renderer* _renderer = nullptr;
};

}
