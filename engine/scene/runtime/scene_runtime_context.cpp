#include "scene_runtime_context.h"

#include "../../io/loaders/asset_config_types.h"

namespace elysia::scene
{
SceneRuntimeContext::SceneRuntimeContext(
    SDL_Renderer* renderer,
    const elysia::io::ContentRegistry& content_registry,
    int logical_width,
    int logical_height,
    const elysia::assist::EngineAssistCache* engine_assist_cache,
    elysia::typography::FontResolver* font_resolver
) noexcept
    : _renderer(renderer)
    , _content_registry(&content_registry)
    , _logical_width(logical_width)
    , _logical_height(logical_height)
    , _engine_assist_cache(engine_assist_cache)
    , _font_resolver(font_resolver)
{
}

SDL_Renderer* SceneRuntimeContext::renderer() const noexcept
{
    return _renderer;
}

const elysia::io::ContentRegistry& SceneRuntimeContext::content_registry() const noexcept
{
    return *_content_registry;
}

int SceneRuntimeContext::logical_width() const noexcept
{
    return _logical_width;
}

int SceneRuntimeContext::logical_height() const noexcept
{
    return _logical_height;
}

const elysia::assist::EngineAssistCache* SceneRuntimeContext::engine_assist_cache() const noexcept
{
    return _engine_assist_cache;
}

elysia::typography::FontResolver* SceneRuntimeContext::font_resolver() const noexcept
{
    return _font_resolver;
}
}
