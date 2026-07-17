#pragma once

#include "../routing/scene_route.h"

namespace elysia::scene::builtin
{
struct SettingsScenePayload
{
    SceneRoute return_route{};
};
}
