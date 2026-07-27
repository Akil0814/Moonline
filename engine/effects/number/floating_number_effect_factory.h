#pragma once

#include "../effect_types.h"
#include "floating_number_glyph_cache.h"

#include <memory>

namespace elysia::typography
{
class FontResolver;
}

namespace elysia::effects
{
class FloatingNumberEffectFactory
{
public:
	void set_font_resolver(const elysia::typography::FontResolver* font_resolver) noexcept;
	void clear_cache() noexcept;

	[[nodiscard]] std::unique_ptr<FloatingNumberEffect> create(
		const FloatingNumberEffectSpawnRequest& request);

private:
	FloatingNumberGlyphCache _glyph_cache;
	const elysia::typography::FontResolver* _font_resolver = nullptr;
};
}
