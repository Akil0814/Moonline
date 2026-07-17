#pragma once

#include "../scene/routing/scene_route.h"

namespace elysia::scene
{
class SceneManager;
}

namespace elysia::application
{
struct ApplicationDescriptor
{
    int logical_width = 1280;
    int logical_height = 720;
    elysia::scene::SceneRoute initial_route{};
};

class IGameModule
{
public:
    virtual ~IGameModule() = default;

    [[nodiscard]] virtual ApplicationDescriptor descriptor() const = 0;
    virtual void register_scenes(elysia::scene::SceneManager& scene_manager) const = 0;
};
}
