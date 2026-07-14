#include "effect_digit_cache.h"

#include "../../localization/localization_manager.h"

namespace elysia::effects
{
elysia::number::DigitCache* EffectDigitCache::digit_cache(EffectDigitColor color)
{
    const std::size_t index = color_index(color);
    if (index >= _digit_caches.size())
        return nullptr;

    SDL_Renderer* renderer = elysia::localization::LocalizationManager::instance()->renderer();
    if (!renderer)
        return nullptr;

    if (_renderer != renderer)
        reset_for_renderer(renderer);

    if (!_configured[index])
    {
        elysia::number::DigitFontSource source;
        source.renderer = renderer;
        source.font_key = "ui.latin.20";
        source.style.point_size = k_point_size;
        source.style.color = color_value(color);
        _digit_caches[index].set_font_source(source);
        _configured[index] = true;
    }

    return &_digit_caches[index];
}

void EffectDigitCache::reset() noexcept
{
    reset_for_renderer(nullptr);
}

elysia::core::Color EffectDigitCache::color_value(EffectDigitColor color) noexcept
{
    switch (color)
    {
    case EffectDigitColor::White: return { 255, 255, 255 };
    case EffectDigitColor::Black: return { 0, 0, 0 };
    case EffectDigitColor::Yellow: return { 255, 220, 48 };
    case EffectDigitColor::Green: return { 80, 220, 100 };
    case EffectDigitColor::Red: return { 230, 70, 70 };
    case EffectDigitColor::Blue: return { 70, 130, 255 };
    case EffectDigitColor::LightBlue: return { 110, 220, 255 };
    case EffectDigitColor::Orange: return { 255, 145, 45 };
    case EffectDigitColor::Purple: return { 185, 105, 255 };
    case EffectDigitColor::Count: break;
    }

    return {};
}

void EffectDigitCache::reset_for_renderer(SDL_Renderer* renderer) noexcept
{
    for (elysia::number::DigitCache& cache : _digit_caches)
        cache.reset();

    _configured.fill(false);
    _renderer = renderer;
}

}
