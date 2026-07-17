#include "scene_runtime_context.h"

#include "../io/loaders/asset_config_types.h"

namespace elysia::scene
{
SceneRuntimeContext::SceneRuntimeContext(
    SDL_Renderer* renderer,
    const elysia::io::ContentRegistry& content_registry,
    int logical_width,
    int logical_height
) noexcept
    : _renderer(renderer)
    , _content_registry(&content_registry)
    , _logical_width(logical_width)
    , _logical_height(logical_height)
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
}
