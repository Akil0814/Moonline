#pragma once

struct SDL_Renderer;

namespace elysia::io
{
struct ContentRegistry;
}

namespace elysia::scene
{
class SceneRuntimeContext
{
public:
    SceneRuntimeContext(
        SDL_Renderer* renderer,
        const elysia::io::ContentRegistry& content_registry,
        int logical_width,
        int logical_height
    ) noexcept;

    [[nodiscard]] SDL_Renderer* renderer() const noexcept;
    [[nodiscard]] const elysia::io::ContentRegistry& content_registry() const noexcept;
    [[nodiscard]] int logical_width() const noexcept;
    [[nodiscard]] int logical_height() const noexcept;

private:
    SDL_Renderer* _renderer = nullptr;
    const elysia::io::ContentRegistry* _content_registry = nullptr;
    int _logical_width = 0;
    int _logical_height = 0;
};
}
